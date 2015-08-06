#include "Skeleton.h"

#include <QtOpenGL>
#include <float.h>

Skeleton::Skeleton()
{
	isReady = false;
	isVisible = false;
	embedMesh = NULL;
	originalStart = originalEnd = -1;

	corr.clear();
}

Skeleton::~Skeleton()
{

}

void Skeleton::saveToFile(const char* fileName)
{
        std::ofstream file(fileName);

        // Skeleton header
        file << "# D:3 NV:" << nodes.size() << " NE:" << edges.size() <<"\n";

        // Save Skeleton Nodes
        for(int i = 0; i < (int)nodes.size(); i++)
                file << "v " << nodes[i].x() << " " << nodes[i].y() << " " << nodes[i].z() << "\n";

        for(int i = 0; i < (int)edges.size(); i++)
                file << "e " << (edges[i].n1->index + 1) << " " << (edges[i].n2->index + 1) << "\n";

        // Correspondence file
        if(v_corr.size() > 0){
            std::string cfileName = fileName;
            cfileName += ".corr.txt";
            std::ofstream cfile(cfileName.c_str());

            for(int i = 0; i < v_corr.size(); i++)
                cfile << v_corr[i] << "\n";
        }
}

void Skeleton::loadFromFile(const char* fileName)
{
	printf("\n\n==================\n");
	printf("Loading Skeleton file...(%s)\n", fileName);

	std::string inputLine;
	FileStream file (fileName);

	double x,y,z;
	int n1, n2;

	int degree;
	int numberOfNodes;
	int numberOfEdges;

	int nCount = 0;
	int nEdges = 0;

	nodes.clear();
	edges.clear();
	lastSelectedFaces.clear();

	if (file.is_open())
	{
		if(!file.eof() && GetLine (file, inputLine))
		{
			if(sscanf(inputLine.c_str(), "# D:%d NV:%d NE:%d", &degree, &numberOfNodes, &numberOfEdges) != 3)
			{
				printf("Error reading skeleton file (check header).");
				printf("\n%d - %d - %d\n", degree, numberOfNodes, numberOfEdges);
				return;
			}
		}

		while (!file.eof())
		{
			GetLine (file, inputLine);

			switch(inputLine[0])
			{
			case 'v':
				if(sscanf(inputLine.c_str(), "v %f %f %f", &x,&y,&z) == 3)
					nodes.push_back( SkeletonNode(x,y,z, nCount++) );
				break;

			case 'e':
				if(sscanf(inputLine.c_str(), "e %d %d", &n1, &n2) == 2)
					edges.push_back( SkeletonEdge(&nodes[n1 - 1], &nodes[n2 - 1], nEdges++) );
				break;
			}
		}

		file.close();

		printf("\nSkeleton file loaded: \n\n\t Nodes: %d \t Edges: %d\n\n", nCount, nEdges);

		postSkeletonLoad();
	}
	else
	{
		printf("\n ERROR: cannot open skeleton file.\n");
	}
}



void Skeleton::loadCorrespondenceFile(const char* fileName)
{
	std::string inputLine;
	FileStream file (fileName);

	printf("Loading correspondence file...");

	int nodeIndex = 0;
	int vIndex = 0;

	corr.clear();

	if (file.is_open())
	{
		while (!file.eof())
		{
			GetLine (file, inputLine);

			if(inputLine.length() > 0 && sscanf(inputLine.c_str(), "%d", &nodeIndex))
			{
				v_corr[vIndex] = nodeIndex - 1;
				corr[v_corr[vIndex]].push_back(vIndex);

				vIndex++;
			}
		}

		file.close();
		printf("done.\n");
	}
	else
	{
		printf("ERROR: cannot open correspondence file.\n");
	}
}

void Skeleton::postSkeletonLoad()
{
	isReady = true;
	isVisible = true;
	isUserFriendly = false;

	// empty selection
	deselectAll();
}

void Skeleton::calculateEdgesLengths()
{
	double len, minEdgeLength = DBL_MAX;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		len = edges[i].calculateLength();

		if(len < minEdgeLength)
			minEdgeLength = len;

		if(len < SkelEpsilon)
			printf("Bad bone! ");
	}
}

SkeletonGraph Skeleton::getGraph()
{
	SkeletonGraph g;

	for(int i = 0; i < (int)edges.size(); i++)
		g.AddEdge(edges[i].n1->index, edges[i].n2->index, edges[i].calculateLength(), edges[i].index);

	return g;
}

void Skeleton::deselectAll()
{
	selectedEdges.clear();
	sortedSelectedNodes.clear();

	smoothNodes.clear();
	smoothEdges.clear();

	lastSelectedFaces.clear();

	selectedNodes = std::vector<bool>(nodes.size(), false);
	selectNodeStart = selectNodeEnd = -1; // clear = -1
}

void Skeleton::selectNode(int index)
{
	if((int)selectedNodes.size() > index && index > -1)
		selectedNodes[index] = true;

	if(selectNodeStart == -1)		
	{
		selectNodeStart = index;
		lastSelectedFaces.clear();

		std::vector<SkeletonEdge*> startNeighbours = nodeNeighbourEdges(selectNodeStart);
		Vec3d startPlaneNormal = startNeighbours.front()->direction().normalized();
	}

	if(selectNodeStart != index)	selectNodeEnd = index; 
}

void Skeleton::selectEdge(int index, int node1_index)
{
	if(index < 0) return;

	// Don't select a selected edge
	for(int i = 0; i < (int)selectedEdges.size(); i++)
		if(selectedEdges[i].index == index) return;

	selectedEdges.push_back(edges[index]);

	// order nodes based on selection direction
	if(edges[index].n1->index != node1_index)
	{
		// swap if necessary
		SkeletonNode * temp = edges[index].n1;
		edges[index].n1 = edges[index].n2;
		edges[index].n2 = temp;
	}
}

void Skeleton::selectEdges(int start, int end)
{
	printf("Selection:  Start (%d), End (%d) \n", selectNodeStart, selectNodeEnd);

	if(selectNodeStart != -1 && selectNodeEnd != -1 && selectNodeStart != selectNodeEnd)
	{
		SkeletonGraph g = getGraph();

		std::list<int> path = g.DijkstraShortestPath(start, end);

		deselectAll();

		int prevNode = *path.begin();

		for(std::list<int>::iterator it = path.begin(); it != path.end(); it++)
		{
			selectNode(*it);
			selectEdge(getEdge(prevNode, *it), prevNode);

			sortedSelectedNodes.push_back(*it);

			prevNode = *it;
		}

		// If we never crop
		if(originalEnd == -1)
		{
			originalStart = selectNodeStart;
			originalEnd = selectNodeEnd;
		}
	}

	originalSelectedNodes = sortedSelectedNodes;
	originalSelectedEdges = selectedEdges;
}

void Skeleton::smoothSelectedEdges(int numSmoothingIterations)
{
	if(!selectedEdges.size())
		return;

	smoothNodes.clear();
	smoothEdges.clear();

	int nCount = 0;
	int eCount = 0;

	SkeletonNode *n1, *n2, *n3;
	n1 = n2 = n3 = NULL;

	smoothNodes.reserve(selectedEdges.size() * 3);
	smoothNodes.push_back(SkeletonNode(*selectedEdges[0].n1, nCount));
	n1 = &smoothNodes[nCount];

	for(int i = 0; i < (int)selectedEdges.size(); i++)
	{
		smoothNodes.push_back(SkeletonNode::Midpoint(n1, selectedEdges[i].n2, nCount + 1));
		smoothNodes.push_back(SkeletonNode(*selectedEdges[i].n2, nCount + 2));

		n2 = &smoothNodes[nCount+1];
		n3 = &smoothNodes[nCount+2];

		smoothEdges.push_back(SkeletonEdge(n1, n2, eCount));	
		smoothEdges.push_back(SkeletonEdge(n2, n3, eCount+1));	

		smoothEdges[eCount].calculateLength();
		smoothEdges[eCount+1].calculateLength();

		nCount += 2;
		eCount += 2;

		n1 = n3;
	}

	// Laplacian smoothing (best option?)
	for(int stage = 0; stage < numSmoothingIterations; stage++)
	{
		std::vector<Vec3d> positions = std::vector<Vec3d>(smoothNodes.size());

		for(int i = 1; i < (int)smoothNodes.size() - 1; i++)
		{
			positions[i] = ((smoothNodes[i-1] + smoothNodes[i+1]) / 2.0) - smoothNodes[i];
			positions[i] = smoothNodes[i] + (positions[i] * 0.25);
		}

		for(int i = 1; i < (int)smoothNodes.size() - 1; i++)
			smoothNodes[i].set(positions[i]);
	}

	// Recompute edge lengths
	for(int i = 0; i < smoothEdges.size(); i++)
		smoothEdges[i].calculateLength();

	// clear old list of selected edges
	selectedEdges.clear();

	// fill in the new smooth edges
	selectedEdges = smoothEdges;
}

void Skeleton::cropSelectedEdges(int start, int end)
{
	originalStart = selectNodeStart;
	originalEnd = selectNodeEnd;

	if(sortedSelectedNodes.size() > 5)
	{
		int N = sortedSelectedNodes.size() - 1;
		selectEdges(sortedSelectedNodes[start], sortedSelectedNodes[N - end]);
	}
}		

std::vector<Vec3d> Skeleton::getSelectedSkeletonPoints()
{
	std::vector<Vec3d> skeletonPoints;

	for(int i = 0; i < (int)selectedEdges.size(); i++)
		skeletonPoints.push_back(*selectedEdges[i].n1);

	skeletonPoints.push_back(*(*selectedEdges.rbegin()).n2); // Last point

	return skeletonPoints;
}

int Skeleton::getEdge(int n1, int n2)
{
	int index = -1;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		if((edges[i].n1->index == n1 && edges[i].n2->index == n2) || 
			(edges[i].n1->index == n2 && edges[i].n2->index == n1))
		{
			return edges[i].index;
		}
	}

	return index;
}

Vec3d Skeleton::selectedEdgesPlane()
{
	if(selectedEdges.size() < 3)
		return Vec3d(0,0,0);
	else
		return cross(selectedEdges[0].direction() , selectedEdges[selectedEdges.size() - 1].direction());
}

std::vector<SkeletonNode *> Skeleton::nodeNeighbours(int node_index)
{
	std::vector<SkeletonNode *> neighbours;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		if(edges[i].n1->index == node_index)
			neighbours.push_back(&nodes[edges[i].n2->index]);
		else if(edges[i].n2->index == node_index)
			neighbours.push_back(&nodes[edges[i].n1->index]);
	}

	return neighbours;
}

std::vector<SkeletonEdge *> Skeleton::nodeNeighbourEdges(int node_index)
{
	std::vector<SkeletonEdge *> neighbours;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		if(edges[i].n1->index == node_index)
			neighbours.push_back(&edges[i]);
		else if(edges[i].n2->index == node_index)
			neighbours.push_back(&edges[i]);
	}

	return neighbours;
}

double Skeleton::nodeRadius(int node_index)
{
	Surface_mesh::Vertex_property<Point> points = embedMesh->vertex_property<Point>("v:point");

	double avgDist = 0;

	Vec3d node_center = nodes[node_index];

	for(int j = 0; j < (int)corr[node_index].size(); j++)
	{
		avgDist += (points[Surface_mesh::Vertex(corr[node_index][j])] - node_center).norm();
	}

	avgDist /= corr[node_index].size();

	return avgDist;
}

Vec3d Skeleton::centerOfNode(SkeletonNode * n)
{
	Surface_mesh::Vertex_property<Point> points = embedMesh->vertex_property<Point>("v:point");

	Vec3d center;

	for(int j = 0; j < (int)corr[n->index].size(); j++)
	{
		center += points[Surface_mesh::Vertex(corr[n->index][j])];
	}

	return center / corr[n->index].size();
}

std::pair< std::vector<int>, std::vector<int> > Skeleton::Split(int edgeIndex)
{
	SkeletonGraph g;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		if(edgeIndex != i)
			g.AddEdge(edges[i].n1->index, edges[i].n2->index, edges[i].length, edges[i].index);
	}

	SkeletonEdge * e = &edges[edgeIndex];

	std::map<int, int> partA, partB;

	// Find the two connected components
	for(int i = 0; i < (int)nodes.size(); i++)
	{
		if(g.isConnected( e->n1->index, nodes[i].index ))
			partA[i] = i;
		else
			partB[i] = i;
	}

	std::vector<int> pointsA, pointsB;


	for(std::map<int,int>::iterator it = partA.begin(); it != partA.end(); it++)
	{
		int i = (*it).first;
		for(int p = 0; p < (int)corr[i].size(); p++)
			pointsA.push_back(corr[i][p]);
	}

	for(std::map<int,int>::iterator it = partB.begin(); it != partB.end(); it++)
	{
		int i = (*it).first;
		for(int p = 0; p < (int)corr[i].size(); p++)
			pointsB.push_back(corr[i][p]);
	}

	return std::make_pair(pointsA, pointsB);
}

void Skeleton::drawNodesNames()
{
	for(int i = 0; i < (int)nodes.size(); i++)
	{
		glPushName(i);
		glBegin(GL_POINTS);
		glVertex3dv(nodes[i]);
		glEnd();
		glPopName();
	}
}

void Skeleton::draw(bool drawMeshPoints)
{
	if(!isVisible || !isReady)
		return;

	if(isUserFriendly)
	{
		//drawUserFriendly();
		return;
	}
	
	glDisable(GL_LIGHTING);
	glEnable(GL_POINT_SMOOTH);

	//======================
	// Draw segmentation
    /*if(drawMeshPoints)
	{
		if(colors.size() != sortedSelectedNodes.size())
			colors = SimpleDraw::RandomColors(sortedSelectedNodes.size());

		int color_id = 0;

		for(std::vector<int>::iterator it = sortedSelectedNodes.begin(); 
			it != sortedSelectedNodes.end(); it++)
		{
			glColor4dv(colors[color_id]);
			glBegin(GL_POINTS);

			for(int i = 0; i < corr[*it].size(); i++)
				glVertex3dv( embedMesh->getVertexPos(corr[*it][i]) );
			
			glEnd();

			color_id++;
		}
    }*/

	glClear(GL_DEPTH_BUFFER_BIT);

	//======================
	// Draw Skeleton Edges

	float oldLineWidth = 0;
	glGetFloatv(GL_LINE_WIDTH, &oldLineWidth);
	glLineWidth(1.5f);

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_LINES);
	for(int i = 0; i < (int)edges.size(); i++)
	{
		int nIndex1 = edges[i].n1->index;
		int nIndex2 = edges[i].n2->index;

		glColor4d(1,1,1,0.2);

		bool isEdgeSelected = false;
		if(selectedNodes[nIndex1] && selectedNodes[nIndex2])
			isEdgeSelected = true;

		if(isEdgeSelected)	glColor3d(0.5, 0.0, 0.2);
		glVertex3d(edges[i].n1->x(), edges[i].n1->y(), edges[i].n1->z());

		if(isEdgeSelected)	glColor3f(1.0f, 0.2f, 0.2f);
		glVertex3d(edges[i].n2->x(), edges[i].n2->y(), edges[i].n2->z());
	}
	glEnd();

	glDisable(GL_BLEND);

	//======================
	// Draw Skeleton Nodes
	for(int i = 0; i < (int)nodes.size(); i++)
	{
		if(selectNodeStart == i)		glColor3d(0.4, 0.0, 0.5);
		else if(selectNodeEnd == i)		glColor3d(0.8, 0.3, 0.95);
		else if(selectedNodes[i])		glColor3d(0.9, 0.2, 0.2);
		else							glColor3d(0.7, 0.7, 0.7);

		glPointSize(7.0f);
		glBegin(GL_POINTS);
		glVertex3f(nodes[i].x(), nodes[i].y(), nodes[i].z());
		glEnd();

		// White Border
		glPointSize(10.0f);
		glColor3f(1,1,1);

		glBegin(GL_POINTS);
		glVertex3f(nodes[i].x(), nodes[i].y(), nodes[i].z());
		glEnd();
	}

	//======================
	// Draw smooth skeletons if any
	glClear(GL_DEPTH_BUFFER_BIT);
	for(int i = 0; i < (int)smoothEdges.size(); i++)
	{
		glLineWidth(2.5f);
		glColor3f(0,0.6f,0);

		SkeletonNode * n1 = smoothEdges[i].n1;
		SkeletonNode * n2 = smoothEdges[i].n2;

		glBegin(GL_LINES);
		glVertex3f(n1->x(), n1->y(), n1->z());
		glVertex3f(n2->x(), n2->y(), n2->z());
		glEnd();

		glPointSize(8.0f);
		glBegin(GL_POINTS);
		glVertex3f(n1->x(), n1->y(), n1->z());
		glVertex3f(n2->x(), n2->y(), n2->z());
		glEnd();

		glPointSize(12.0f);
		glColor3f(0.8f, 0.9f, 0.8f);
		glBegin(GL_POINTS);
		glVertex3f(n1->x(), n1->y(), n1->z());
		glVertex3f(n2->x(), n2->y(), n2->z());
		glEnd();
	}

	glLineWidth(oldLineWidth);
	glColor3f(1,1,1);

	glEnable(GL_LIGHTING);
}

std::vector<uint> Skeleton::getSelectedFaces(bool growSelection)
{
	std::set<uint> faceResultSet;
	int numVerticesSelected = 0;

	SkeletonGraph g = getGraph();

	std::set<int> activeNodes;
	activeNodes.insert(originalSelectedNodes.front());
	activeNodes.insert(originalSelectedNodes.back());

	if(originalSelectedNodes.size() > 1)
	{
		// Explore branches, help us deal with branchy skeletons
		g.explore(originalSelectedNodes[1], activeNodes);
	}

	// If we need a grown selection, we add the neighbor's nodes at the end points
    /*if(growSelection)
	{
		std::set<int> startNeighbours = VECTOR_TO_SET(g.GetNeighbours(originalSelectedNodes.front()));
		foreach(int n, startNeighbours) activeNodes.insert(n);

		std::set<int> endNeighbours = VECTOR_TO_SET(g.GetNeighbours(originalSelectedNodes.back()));
		foreach(int n, endNeighbours)	activeNodes.insert(n);

		foreach(int i, endNeighbours) 
		{
			std::vector<int> NN = g.GetNeighbours(i);
			foreach(int j, NN) activeNodes.insert(j);
		}
    }*/

    /*foreach(int n, activeNodes)
	{
		numVerticesSelected += corr[n].size();

		for(int v = 0; v < (int)corr[n].size(); v++)
		{
			int vIndex = corr[n][v];

			std::set<uint> adjF = embedMesh->faceIndicesAroundVertex(embedMesh->vertex_array[vIndex]);

			foreach(uint index, adjF)
				faceResultSet.insert(index);
		}
    }*/

	// Convert to vector
    std::vector<uint> result;// = SET_TO_VECTOR(faceResultSet);

	return (lastSelectedFaces = result);
}

std::vector<ResampledPoint> Skeleton::resampleSmoothSelectedPath( int numSteps /*= 20*/, int smoothSteps /*= 3*/ )
{
	std::vector<ResampledPoint> result;

	smoothSelectedEdges(smoothSteps);

	// Compute length of each segment
	double totalLength = 0;
	foreach(SkeletonEdge e, smoothEdges)
		totalLength += e.length;
	double segmentLength = totalLength / numSteps;

	// Start at
	double t =  0;
	int index = 0;

	// Compute equal-dist points on polyline
	for(int s = 0; s <= numSteps; s++)
	{
		// Which node does it belong too (based on distance)
		int nid = smoothEdges[index].n1->index;
		if( t > 0.5 ) nid = smoothEdges[index].n2->index;

		// Add sample
		result.push_back(ResampledPoint(smoothEdges[index].pointAt(t), nid));

		if(!walkSmoothEdges(segmentLength, t, index, t, index))
		{
			Vec3d delta = result.back().pos - result[result.size() - 2].pos;
			result.push_back(ResampledPoint(delta + result.back().pos, nid));
			break;
		}
	}

	return result;
}

int Skeleton::walkSmoothEdges(double distance, double startTime, int index, double & destTime, int & destIndex)
{
	double remain = smoothEdges[index].lengthsAt(startTime).second;

	// Case 1: the point is on the starting line
	if(remain > distance)
	{
		double startLength = startTime * smoothEdges[index].length;
		destTime = (startLength + distance) / smoothEdges[index].length;
		destIndex = index;
		return 1;
	}

	double walked = remain;

	// Case 2: keep walking next lines
	while(walked < distance)
	{
		index = index + 1;		// step to next line

		if(index + 1 == smoothEdges.size())
			return 0;

		walked += smoothEdges[index].length;
	}

	// Step back to the start of this line
	walked -= smoothEdges[index].length;

	double remainDistance = distance - walked;
	double endTime = remainDistance / smoothEdges[index].length;

	destTime = endTime;
	destIndex = index;

	return 1;
}

void Skeleton::selectLongestPath()
{
	std::list<int> path = getGraph().GetLargestConnectedPath();

	if(path.size())
	{
		selectNode(path.front());	
		selectNode(path.back());
		selectEdges(path.front(), path.back());
	}
	else
	{
		// Something went wrong, select stuff anyway..
		selectNode(nodes.front().index);
		selectNode(nodes.back().index);
		selectEdges(nodes.front().index,nodes.back().index);
	}
}
