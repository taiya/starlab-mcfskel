#pragma once

#include "SkeletonNode.h"
#include "SkeletonEdge.h"

#include "Graph.h"

typedef Graph<int, double> SkeletonGraph;

class Skeleton
{

public:
	Skeleton();
	~Skeleton();

	bool isReady;
	bool isVisible;
	bool isUserFriendly;

	// STRUCTURE
	std::vector<SkeletonNode> nodes;
	std::vector<SkeletonEdge> edges;
	std::map<int, std::vector<int> > corr;	// Node -> vertex indices
	std::map<int, int> v_corr;				// vertex index -> Node
	std::map<int, int> f_corr;				// face index -> Node

    // DATA LOADING / SAVING
	void loadFromFile(const char* fileName);
	void loadCorrespondenceFile(const char* fileName);
	void postSkeletonLoad();
    void saveToFile(const char* fileName);

	// COMPUTATIONS
	void calculateEdgesLengths();
	SkeletonGraph getGraph();
	void selectLongestPath();

	// SELECTION
	std::vector<SkeletonEdge> originalSelectedEdges;
	std::vector<SkeletonEdge> selectedEdges;
	std::vector<bool> selectedNodes;
	std::vector<int> sortedSelectedNodes;
	std::vector<int> originalSelectedNodes;
	int selectNodeStart;
	int selectNodeEnd;
	int originalStart;
	int originalEnd;
	double minEdgeLength;

	void selectNode(int index);
	void selectNodeFromFace(int index);
	void selectEdge(int index, int node1_index);
	void selectEdges(int start, int end);
	void deselectAll();

	Vec3d selectedEdgesPlane();

	// NODES FUNCTIONS
	std::vector<SkeletonNode*> nodeNeighbours(int node_index);
	std::vector<SkeletonEdge*> nodeNeighbourEdges(int node_index);
	double nodeRadius(int node_index);
	Vec3d centerOfNode(SkeletonNode * n);

	// EDGE FUNCTIONS
	int getEdge(int n1, int n2);

	// GET POINTS
	std::vector<Vec3d> getSelectedSkeletonPoints();

	// FACE SELECTION
	std::vector<uint> getSelectedFaces(bool growSelection);
	std::vector<uint> lastSelectedFaces;

	// SMOOTH EDGES
	std::vector<SkeletonEdge> smoothEdges;
	std::vector<SkeletonNode> smoothNodes;
	void smoothSelectedEdges(int numSmoothingIterations = 3);
	void cropSelectedEdges(int start = 1, int end = 1);
	std::vector<ResampledPoint> resampleSmoothSelectedPath(int numSteps = 20, int smoothSteps = 3);
	int walkSmoothEdges(double distance, double startTime, int index, double & destTime, int & destIndex);
	// MODIFY OPERATIONS
	std::pair< std::vector<int>, std::vector<int> > Split(int edgeIndex);

	// EMBEDDING
    SurfaceMeshModel * embedMesh;

	// RENDERING FOR SELECTION
	void drawNodesNames();
	void drawMeshFacesNames();

	// VISUALIZATION
	void draw(bool drawMeshPoints = false);
	std::vector<Color> colors;

	// DEBUG:
	std::set<int> testNodes;
};

// IO name decorations
#include <fstream>
#define FileStream std::ifstream 
#define FileStreamOutput std::ofstream 
#define GetLine std::getline 

#define SkelEpsilon  ( std::numeric_limits<double>::epsilon() * 10 ) 
