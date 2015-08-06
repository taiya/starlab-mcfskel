
#include "SkeletonExtract.h"
#include <QElapsedTimer>
#include <cmath>

// from stackoverflow.com
bool is_nan(double var){
	volatile double d = var;
	return d != d;
}

SkeletonExtract::SkeletonExtract( QSurfaceMesh * fromMesh, double laplacianWeight, double laplacianScale, 
	double posiWeight, double posScale, double origPosWeight, double shapeEnergyWeight, double postSimplifyErrRatio, bool isApplyJointMerging)
{
	// Initialize parameters:
	LaplacianConstraintWeight = laplacianWeight;
	PositionalConstraintWeight = posiWeight;
	OriginalPositionalConstraintWeight = origPosWeight;
	LaplacianConstraintScale = laplacianScale;
	PositionalConstraintScale = posScale;
	ShapeEnergyWeight = shapeEnergyWeight;
	PostSimplifyErrorRatio = postSimplifyErrRatio;

	faceAreaThreshold = 0.000000001; // numerical issues..
	volumneRatioThreashold = 0.00001;
	isApplyJointMergingStep = isApplyJointMerging; // usefulness?

	// Use a copy of the mesh
    //this->src_mesh = fromMesh;
    this->mesh = fromMesh;

	// Normalize
    this->scaleFactor = this->mesh->normalize();
    this->mesh->computeBoundingBox();

	// Element count
    n = mesh->n_vertices();
    fn = mesh->n_faces();

	// Compute areas of mesh faces
	originalFaceArea.resize(fn);

	mesh->assignVertexArray();
	mesh->assignFaceArray();
	mesh->fillTrianglesList();

	// Compute face areas
	originalArea = 0;
	for (int i = 0; i < fn; i++){
        originalFaceArea[i] = mesh->faceArea(mesh->face_array[i]);
		originalArea += originalFaceArea[i];
	}

	// Compute mesh volume
    originalVolume = mesh->volume();
    originalVertexPos = mesh->clonePoints();

    LaplacianConstraintWeight = 1.0 / (10.0 * sqrt(originalArea/ mesh->n_faces()));

	// per-vertex
	lapWeight.resize(n, LaplacianConstraintWeight);
	posWeight.resize(n, PositionalConstraintWeight);
	collapsedLength.resize(n, 0);
	vertexFlag.resize(n, 0);

	// Simplification parameters
	TargetVertexCount = 10;

	// this needs to be changed to pure Surface_mesh instead..
	SetupLocalAdjacenciesLists();

	// Steps:
	//  1) Geometry Collapse
	this->GeometryCollapse();

	//  2) Simplification
	this->Simplification();

	//  3) Improve Embedding
	this->EmbeddingImproving();

	// Transformed back into world space
	for(uint i = 0; i < collapsedVertexPos.size(); i++)
		this->collapsedVertexPos[i] *= this->scaleFactor;

	for(uint i = 0; i < simplifiedVertexPos.size(); i++)
		simplifiedVertexPos[i] *= this->scaleFactor;

	for(uint i = 0; i < embededVertexPos.size(); i++)
		embededVertexPos[i] *= this->scaleFactor;

	// Restore original mesh
	Surface_mesh::Vertex_property<Point> points = mesh->vertex_property<Point>("v:point");
	Surface_mesh::Vertex_iterator vit, vend = mesh->vertices_end();

	for (vit = mesh->vertices_begin(); vit != vend; ++vit)
		points[vit] *= this->scaleFactor;

	debugPoints = simplifiedVertexPos;
	debugPoints2 = embededVertexPos;
}

void SkeletonExtract::GeometryCollapse(int maxIter)
{
	double currentVolume = 0;
	double currentArea = 0;

	int iteration = 0;

	do{
		QElapsedTimer timer; timer.start();

		// Build contraction matrix
		A = BuildMatrixA();
		ATA = A.transpose() * A;

		// Apply smooth operation
		ImplicitSmooth();
	
		// Compute new mesh volume
        currentVolume = mesh->volume();

		// Compute new mesh area
		currentArea = 0;
		for (int i = 0; i < fn; i++) 
            currentArea += mesh->faceArea(mesh->face_array[i]);

		// Step done
		iteration++;

		printf("(%d) Area: %f \t Vol: %f \t CPU Time: %d (ms) \n", iteration,  
			currentArea / originalArea, currentVolume / originalVolume, timer.elapsed());
	}
	while (currentVolume / originalVolume > volumneRatioThreashold && iteration < maxIter);

	// Resulting collapsed positions
    this->collapsedVertexPos = mesh->clonePoints();
}

#define Max(a,b) (((a) > (b)) ? (a) : (b))
#define Min(a,b) (((a) < (b)) ? (a) : (b))

Eigen::SparseMatrix<double> SkeletonExtract::BuildMatrixA()
{
    Surface_mesh::Vertex_property< std::set<uint> > adjVF = mesh->vertex_property< std::set<uint> >("v:adjVF");

    // Use dynamic sparse matrix only when filling
    Eigen::SparseMatrix<double> matA(3 * n, n);

	std::vector<double> areaRatio (fn);
	std::vector<double> collapsed (n);

	// Max weight parameters
	double MAX_POS_WEIGHT = 10000;
	double MAX_LAP_WEIGHT = 2048;

	// For each face on mesh, compute areas and cots
	for (int i = 0; i < fn; i++)
	{
        double newAreaFace = abs(mesh->faceArea(mesh->face_array[i]));
		areaRatio[i] = newAreaFace / originalFaceArea[i];

		// Indices
        uint f_id = Surface_mesh::Face(mesh->face_array[i]).idx();
        uint c1 = mesh->triangles[(f_id * 3) + 0];
        uint c2 = mesh->triangles[(f_id * 3) + 1];
        uint c3 = mesh->triangles[(f_id * 3) + 2];

        Point v1 = mesh->getVertexPos(c1);
        Point v2 = mesh->getVertexPos(c2);
        Point v3 = mesh->getVertexPos(c3);

		double cot1 = dot(v2 - v1, v3 - v1) / cross(v2 - v1, v3 - v1).norm();
		double cot2 = dot(v3 - v2, v1 - v2) / cross(v3 - v2, v1 - v2).norm();
		double cot3 = dot(v1 - v3, v2 - v3) / cross(v1 - v3, v2 - v3).norm();

		if(is_nan(cot1) || is_nan(cot2) || is_nan(cot3) || newAreaFace < faceAreaThreshold)
			cot1 = cot2 = cot3 = 0;

		// Assign to sparse matrix
		matA.coeffRef(c2, c2) += -cot1; matA.coeffRef(c2, c3) += cot1;
		matA.coeffRef(c3, c3) += -cot1; matA.coeffRef(c3, c2) += cot1;
		matA.coeffRef(c3, c3) += -cot2; matA.coeffRef(c3, c1) += cot2;
		matA.coeffRef(c1, c1) += -cot2; matA.coeffRef(c1, c3) += cot2;
		matA.coeffRef(c1, c1) += -cot3; matA.coeffRef(c1, c2) += cot3;
		matA.coeffRef(c2, c2) += -cot3; matA.coeffRef(c2, c1) += cot3;
	}

	// For each vertex
	for (int i = 0; i < n; i++)
	{
		double totalRatio = 0;
	
		// For all neighbor faces of v_i, get average area ratios
        std::set<uint> adjF = adjVF[mesh->vertex_array[i]];
		foreach(uint fi, adjF) totalRatio += areaRatio[fi];
		totalRatio /= adjF.size();

		double totalPosWeight = matA.col(i).sum();

		if (totalPosWeight > MAX_POS_WEIGHT)
		{
			collapsed[i] = true;
			vertexFlag[i] = 1;

			matA.col(i) /= MAX_POS_WEIGHT;
		}

		// normalized by row sum
		matA.col(i) *= lapWeight[i];

		// Then assign new weights
		lapWeight[i] *= LaplacianConstraintScale;
		posWeight[i] = (1.0 / (sqrt(totalRatio))) * PositionalConstraintWeight;
	
		lapWeight[i] = Min(MAX_LAP_WEIGHT, lapWeight[i]);
		posWeight[i] = Min(MAX_POS_WEIGHT, posWeight[i]);
	}

	// Assign positional weights
	for (int i = 0; i < n; i++)
	{
		matA.coeffRef(i + n, i) = posWeight[i];
		matA.coeffRef(i + n + n, i) = OriginalPositionalConstraintWeight;
	}

    // Return as regular SparseMatrix
    return Eigen::SparseMatrix<double> (matA);
}

void SkeletonExtract::ImplicitSmooth()
{
	Eigen::VectorXd ATb, x(n);
	Eigen::VectorXd b (n * 3);

    std::vector<Point> newPos = mesh->clonePoints();
	std::vector<Point> oldPos = newPos;

    Eigen::SimplicialLLT< Eigen::SparseMatrix<double> > solver;
    solver.compute(ATA);

	// for each of axis 'x' 'y' 'z'
	for (uint i = 0; i < 3; i++)
	{
		// Assign values
		for (uint j = 0; j < n; j++){
			b(j) = 0;
			b(j + n) = oldPos[j][i] * posWeight[j];
			b(j + n + n) = 0;
		}

		// Solve A * x = b
		ATb = A.transpose() * b;
        x = solver.solve(ATb);

		for (uint j = 0; j < n; j++)
			newPos[j][i] = x[j];
	}

    mesh->setFromPoints(newPos);
}

void SkeletonExtract::Simplification()
{
	QElapsedTimer timer; timer.start();

    Surface_mesh::Vertex_iterator vit, vend = mesh->vertices_end();

    for (vit = mesh->vertices_begin(); vit != vend; ++vit)
        vRec.push_back( VertexRecord(*mesh, vit) );

	vRec_original = vRec; // make a copy (used later for face correspondence)

	// init weights
	for (uint i = 0; i < n; i++)
	{
		Vec3d p1 = vRec[i].pos;
		vRec[i].minError = std::numeric_limits<double>::max();
		vRec[i].minIndex = -1;

		foreach (uint j, vRec[i].adjV)
		{
			Vec3d p2 = vRec[j].pos;
			Vec3d u = (p2 - p1).normalized();
			Vec3d w = cross(u, p1);
			Matrix4d m = Matrix4d::Zero();
			m(0, 1) = -u.z(); m(0, 2) =  u.y(); m(0, 3) = -w.x();
			m(1, 0) =  u.z(); m(1, 2) = -u.x(); m(1, 3) = -w.y();
			m(2, 0) = -u.y(); m(2, 1) =  u.x(); m(2, 3) = -w.z();
			vRec[i].matrix += m.transpose() * m;
		}

		UpdateVertexRecords(vRec[i]);
	}
	
	// Put record into priority queue
	//std::priority_queue<VertexRecord> queue;
	PriorityQueue queue;
	for (uint i = 0; i < n; i++)
		queue.Insert(&vRec[i]);

    int facesLeft = mesh->n_faces();
    int vertexLeft = mesh->n_vertices();

	remainingVertexCount = vertexLeft;

	// Simplify
	while (facesLeft > 0 && vertexLeft > TargetVertexCount && !queue.IsEmpty())
	{
		VertexRecord * rec1 = &vRec[queue.DeleteMin()->vIndex];
		VertexRecord * rec2 = &vRec[rec1->minIndex];

		rec2->matrix = (rec1->matrix + rec2->matrix);

		if (rec1->center)
			rec2->pos = (rec1->pos + rec2->pos) / 2.0;

		rec2->collapseFrom.insert(rec1->vIndex);

		foreach (uint index, rec1->collapseFrom) 
			rec2->collapseFrom.insert(index);

		rec1->collapseFrom.clear();

		uint r1 = rec1->vIndex;
		uint r2 = rec2->vIndex;

		int count = 0;

		// For all neighboring faces
		foreach (uint index, rec1->adjF)
		{
            uint c1 = mesh->triangles[(index * 3) + 0];
            uint c2 = mesh->triangles[(index * 3) + 1];
            uint c3 = mesh->triangles[(index * 3) + 2];

			if ((c1 == r2 || c2 == r2 || c3 == r2) || 
				(c1 == c2 || c2 == c3 || c3 == c1))
			{
				// remove adj faces
				foreach (uint index2, rec1->adjV)
					vRec[index2].adjF.erase(index);

				facesLeft--;
				count++;
			}
			else
			{
				// update face index
                if (c1 == r1) mesh->triangles[(index * 3) + 0] = r2;
                if (c2 == r1) mesh->triangles[(index * 3) + 1] = r2;
                if (c3 == r1) mesh->triangles[(index * 3) + 2] = r2;

				// add adj faces
				rec2->adjF.insert(index);
			}
		}

		// Fix adj vertices
		foreach (uint index, rec1->adjV)
		{
			VertexRecord * recAdj = &vRec[index];

			if (recAdj->adjV.find(r1) != recAdj->adjV.end())
				recAdj->adjV.erase(r1);

			if (index != r2)
			{
				recAdj->adjV.insert(r2);
				rec2->adjV.insert(index);
			}
		}

		// Update records
		foreach (uint index, rec2->adjV)
		{
			UpdateVertexRecords(vRec[index]);
			queue.Update(&vRec[index]);
		}
		UpdateVertexRecords(*rec2);
		queue.Update(rec2);

		foreach (uint index, rec2->adjF)
		{
            uint c1 = mesh->triangles[(index * 3)];
            uint c2 = mesh->triangles[(index * 3) + 1];
            uint c3 = mesh->triangles[(index * 3) + 2];

			if (c1 == c2 || c2 == c3 || c3 == c1)
			{
				rec2->adjF.erase(index);

				foreach (uint index2, rec2->adjV)
					vRec[index2].adjF.erase(index);

				facesLeft--;
			}
		}

		// Decrease vertex count
		vertexLeft--;
		remainingVertexCount = vertexLeft;
		
		// Debug
		if(facesLeft % 1000 == 0) printf(".");
		if(facesLeft % 5000 == 0) printf(".%d.", facesLeft);
	}

	// Collect remaining vertices
	while (!queue.IsEmpty())
	{
		simplifiedVertexRec.push_back(vRec[queue.DeleteMin()->vIndex]);
		simplifiedVertexPos.push_back(simplifiedVertexRec.back().pos);
	}

	printf("\nWe got (%u) vertices. Simplification took %d(ms).\n", simplifiedVertexPos.size(), timer.elapsed());
}

void SkeletonExtract::UpdateVertexRecords(VertexRecord & rec1)
{
	Vec3d p1 = rec1.pos;

	rec1.minError = std::numeric_limits<double>::max();
	rec1.minIndex = -1;

	// Do not allow collapse at the end of skeleton
	if (rec1.adjV.size() > 1 && rec1.adjF.size() > 0)
	{
		// Sum edge lengths
		double totLength = 0;
		foreach (uint j, rec1.adjV) totLength += (p1 - vRec[j].pos).norm();
		totLength /= rec1.adjV.size();

		foreach (uint j, rec1.adjV)
		{
			// Search for 'j' in my neighboring faces
			bool found = false;
			foreach (uint index, rec1.adjF)
			{
                uint c1 = mesh->triangles[(index * 3)];
                uint c2 = mesh->triangles[(index * 3) + 1];
                uint c3 = mesh->triangles[(index * 3) + 2];

				if (c1 == j || c2 == j || c3 == j)
					found = true;
			}
			if (!found) continue;

			VertexRecord rec2 = vRec[j];
			if (rec2.adjF.size() == 0) continue;

			Vec3d p2 = rec2.pos;

			// Use sampling energy
			double err = 0;
			err = (p1 - p2).norm() * totLength;

			// Use shape energy
			Vec4d v1 (p2.x(), p2.y(), p2.z(), 1.0); 
			Vec3d mid((p1 + p2) / 2.0);
			Vec4d v2 (mid.x(), mid.y(), mid.z(), 1.0);

			Matrix4d m = rec1.matrix + rec2.matrix;

			double e1 = dot(v1, EIGEN4_TO_VEC( (m * VEC_TO_EIGEN4(v1)) ) ) * ShapeEnergyWeight;
			double e2 = dot(v2, EIGEN4_TO_VEC( (m * VEC_TO_EIGEN4(v2)) ) ) * ShapeEnergyWeight;

			if (e1 < e2)
				err += e1;
			else
			{
				err += e2;
				rec1.center = true;
			}

			if (err < rec1.minError){
				rec1.minError = err;
				rec1.minIndex = j;
			}
		}
	}
}

void SkeletonExtract::EmbeddingImproving()
{
    Surface_mesh::Vertex_property< std::set<uint> > adjVV = mesh->vertex_property< std::set<uint> >("v:adjVV");
    //Surface_mesh::Vertex_property< std::set<uint> > adjVF = mesh->vertex_property< std::set<uint> >("v:adjVF");

	QVector<uint> segmentIndex (n);
	QVector<bool> marked (n);

	// init local variables
	for (int i = 0; i < n; i++) marked[i] = false;
	for(uint i = 0; i < simplifiedVertexRec.size(); i++)
	{
		VertexRecord * rec = &simplifiedVertexRec[i];
		rec->collapseFrom.insert(rec->vIndex);

		foreach (int index, rec->collapseFrom)
			segmentIndex[index] = rec->vIndex;
	}

	// for each skeletal node
	for(int i = 0; i < (int) simplifiedVertexRec.size(); i++)
	{
		VertexRecord * rec = &simplifiedVertexRec[i];

		Vec3d totDis (0,0,0);
		rec->pos = originalVertexPos[rec->vIndex];

		if (rec->adjV.size() == 2)
		{
			// for each adjacent node
			foreach (int adj, rec->adjV)
			{
				QSet<int> boundaryVertices;
				Vec3d dis (0,0,0), q (0,0,0);
				double totLen = 0;

				foreach (uint i, rec->collapseFrom)
				{
					foreach (uint j, adjVV[V(i)])
					{
						if (segmentIndex[j] == adj)
						{
							marked[i] = true;
							boundaryVertices.insert(i);
							break;
						}
					}
				}

				foreach (int i, boundaryVertices)
				{
					Vec3d p1 (originalVertexPos[i]);
                    Vec3d p2 (mesh->getVertexPos(i));
					
					double len = 0;
					foreach (uint j, adjVV[V(i)])
					{
						if (marked[j])
						{
							Vec3d u (originalVertexPos[j]);
							len += (p1 - u).norm();
						}
					}

					q += p2 * len;
					dis += (p1 - p2) * len;
					totLen += len;
				}

				foreach (uint i, boundaryVertices) 
					marked[i] = false;

				Vec3d center = (q + dis) / totLen;
				if (totLen > 0) totDis += center;
			}

			rec->pos = totDis / rec->adjV.size();
		}
		else
		{
			Vec3d dis (0,0,0);
			double totLen = 0;
			foreach (uint i, rec->collapseFrom)
			{
				foreach (uint j, adjVV[V(i)])
				{
					if (segmentIndex[j] != rec->vIndex)
						marked[i] = true;
				}
			}
			foreach (uint i, rec->collapseFrom)
			{
				if (!marked[i]) continue;

				Vec3d p1 (originalVertexPos[i]);
                Vec3d p2 (mesh->getVertexPos(i));

				double len = 0;
				foreach (int j, adjVV[V(i)])
				{
					if (marked[j])
					{
						Vec3d u (originalVertexPos[j]);
						len += (p1 - u).norm();
					}
				}

				dis += (p1 - rec->pos) * len;
				totLen += len;
			}

			if (totLen > 0) 
				rec->pos += dis / totLen;
		}

		embededVertexPos.push_back(rec->pos);
	}

	if(isApplyJointMergingStep) MergeJoint();
}

void SkeletonExtract::MergeJoint()
{
    Surface_mesh::Vertex_property< std::set<uint> > adjVV = mesh->vertex_property< std::set<uint> >("v:adjVV");
    Surface_mesh::Vertex_property< std::set<uint> > adjVF = mesh->vertex_property< std::set<uint> >("v:adjVF");

	std::vector<uint> segmentIndex (n);

	// init segment index
	for(uint i = 0; i < simplifiedVertexRec.size(); i++)
	{
		VertexRecord * rec = &simplifiedVertexRec[i];

		rec->collapseFrom.insert(rec->vIndex);

		foreach (int index, rec->collapseFrom)
			segmentIndex[index] = rec->vIndex;
	}

	bool updated = false;

	do
	{
		for(uint i = 0; i < simplifiedVertexRec.size(); i++)
		{
			VertexRecord * rec = &simplifiedVertexRec[i];

			if (!rec->collapseFrom.size()) continue;
			if (rec->adjV.size() <= 2) continue;

			Vec3d p = rec->pos;
			updated = false;

			// compute radius
			double radius = 0;
			foreach (uint index, rec->collapseFrom)
			{
				Vec3d q (originalVertexPos[index]);
				radius += (p - q).norm();
			}
			radius /= rec->collapseFrom.size();

			// compute sd
			double sd = 0;
			foreach (uint index, rec->collapseFrom)
			{
				Vec3d q (originalVertexPos[index]);
				double diff = (p - q).norm() - radius;
				sd += diff * diff;
			}

			sd /= rec->collapseFrom.size();
			sd = sqrt(sd);
			sd /= radius;

			Vec3d minCenter (0,0,0);
			double minSD = std::numeric_limits<double>::max();
			double minRadius = std::numeric_limits<double>::max();
			int minAdj = -1;
			foreach (uint adj, rec->adjV)
			{
				VertexRecord * rec2 = &vRec[adj];
				if (rec2->adjV.size() == 1) continue;
				Vec3d newCenter (0,0,0);
				double newRadius = 0;
				double newSD = 0;

				// compute new center
				Vec3d dis(0,0,0);
				double totLen = 0;
				QSet<uint> marked;

				foreach (uint i, rec->collapseFrom)
					foreach (uint j, adjVV[V(i)])
					if (segmentIndex[j] != rec->vIndex && segmentIndex[j] != adj)
						marked.insert(i);
				foreach (int i, marked)
				{
					Vec3d p1 (originalVertexPos[i]);
                    Vec3d p2 (mesh->getVertexPos(i));
					double len = 0;
					foreach (uint j, adjVV[V(i)])
					{
						if (marked.contains(j))
						{
							Vec3d u (originalVertexPos[j]);
							len += (p1 - u).norm();
						}
					}

					dis += p1 * len;
					totLen += len;
				}

				marked.clear();

				foreach (uint i, rec2->collapseFrom)
				{
					foreach (uint j, adjVV[V(i)])
					{
						if (segmentIndex[j] != rec2->vIndex && segmentIndex[j] != rec->vIndex)
							marked.insert(i);
					}
				}

				foreach (uint i, marked)
				{
					Vec3d p1 (originalVertexPos[i]);
                    Vec3d p2 (mesh->getVertexPos(i));
					double len = 0;
					foreach (uint j, adjVV[V(i)])
					{
						if (marked.contains(j))
						{
							Vec3d u (originalVertexPos[j]);
							len += (p1 - u).norm();
						}
					}

					dis += p1 * len;
					totLen += len;
				}

				newCenter = dis / totLen;

				// compute new radius
				foreach (uint index, rec->collapseFrom)
				{
					Vec3d q (originalVertexPos[index]);
					newRadius += (newCenter - q).norm();
				}
				foreach (uint index, rec2->collapseFrom)
				{
					Vec3d q (originalVertexPos[index]);
					newRadius += (newCenter - q).norm();
				}
				newRadius /= (rec->collapseFrom.size() + rec2->collapseFrom.size());

				// compute sd
				foreach (uint index, rec->collapseFrom)
				{
					Vec3d q (originalVertexPos[index]);
					double diff = (newCenter - q).norm() - newRadius;
					newSD += diff * diff;
				}
				foreach (uint index, rec2->collapseFrom)
				{
					Vec3d q (originalVertexPos[index]);
					double diff = (newCenter - q).norm() - newRadius;
					newSD += diff * diff;
				}
				newSD /= (rec->collapseFrom.size() + rec2->collapseFrom.size());
				newSD = sqrt(newSD);
				newSD /= newRadius;

				if (newSD < minSD)
				{
					minSD = newSD;
					minRadius = newRadius;
					minCenter = newCenter;
					minAdj = adj;
				}
			}

			// merge node if new SD is smaller
			if (minAdj != -1 && minSD < PostSimplifyErrorRatio * sd)
			{
				uint r1 = rec->vIndex;
				VertexRecord * rec2 = &vRec[minAdj];
				rec2->pos = minCenter;
				rec2->collapseFrom.insert(rec->vIndex);
				foreach (uint index, rec->collapseFrom) rec2->collapseFrom.insert(index);
				rec->collapseFrom.clear();

				foreach (uint index, rec->adjV)
				{
					VertexRecord * recAdj = &vRec[index];
					if (recAdj->adjV.find(r1) != recAdj->adjV.end())
						recAdj->adjV.erase(r1);
					if (index != minAdj)
					{
						recAdj->adjV.insert(minAdj);
						rec2->adjV.insert(index);
					}
				}

				updated = true;
			}
		}
	} while (updated);

	QVector<VertexRecord> updatedVertexRec;

	for(uint i = 0; i < simplifiedVertexRec.size(); i++){
		VertexRecord * rec = &simplifiedVertexRec[i];

		if (rec->collapseFrom.size())
			updatedVertexRec.push_back(*rec);
	}

	simplifiedVertexRec = updatedVertexRec;
}

void SkeletonExtract::SetupLocalAdjacenciesLists()
{
    Surface_mesh::Vertex_property< std::set<uint> > adjVV = mesh->vertex_property< std::set<uint> >("v:adjVV");
    Surface_mesh::Vertex_property< std::set<uint> > adjVF = mesh->vertex_property< std::set<uint> >("v:adjVF");

    Surface_mesh::Vertex_iterator vit, vend = mesh->vertices_end();

    for (vit = mesh->vertices_begin(); vit != vend; ++vit)
	{
        adjVV[vit] = mesh->vertexIndicesAroundVertex(vit);
        adjVF[vit] = mesh->faceIndicesAroundVertex(vit);
	}
}

void SkeletonExtract::SaveToSkeleton( Skeleton * s )
{
	printf("\nSaving skeleton..");

	Graph<uint, double> edgesGraph;
	std::map<uint, uint> vMap;

	uint numNodes = simplifiedVertexRec.size();

	for(uint i = 0; i < numNodes; i++)
	{
		VertexRecord * rec = &simplifiedVertexRec[i];
		
		// add its edges
		foreach (uint index, rec->adjV)
			edgesGraph.AddEdge(rec->vIndex, index, 1);

		// re-mapping index
		vMap[rec->vIndex] = i;

		// insert node
		Point p = rec->pos * scaleFactor;
		s->nodes.push_back(SkeletonNode(p.x(), p.y(), p.z(), i));
		
		// Vertex correspondence
		foreach(uint vi, rec->collapseFrom)
		{
			s->v_corr[vi] = i;
			s->corr[i].push_back(vi);

			// Face correspondence (overwrite as you go)
			foreach(uint fi, vRec_original[vi].adjF)
				s->f_corr[fi] = i;
		}
	}

	// Extract unique edges from this undirected graph
	Graph<uint, double>::EdgesSet edges = edgesGraph.GetEdgesSet();

	// Add unique edges
	uint nEdges = 0;
	for(Graph<uint, double>::EdgesSet::iterator e = edges.begin(); e != edges.end(); e++)
	{
		SkeletonNode * n1 = &s->nodes[vMap[e->target]];
		SkeletonNode * n2 = &s->nodes[vMap[e->index]];

		s->edges.push_back(SkeletonEdge(n1, n2, nEdges++));
	}
	s->embedMesh = src_mesh;
	s->postSkeletonLoad();

	printf("Done.\n");
}
