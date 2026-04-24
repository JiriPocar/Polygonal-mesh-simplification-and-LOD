/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Simplificator.hpp
 * @brief Simplification entry point.
 *
 * This file implements the Simplificator class, which serves as the main
 * entry point for mesh simplification operations.
*/

#pragma once

#include "../resources/Model.hpp"

#define DONT_DECIMATE_ERROR 1e9

enum class Algorithm {
	QEM,
	VertexClustering,
	FloatingCellClustering,
	VertexDecimation,
	Naive,
	Random
};

enum class ClusteringMethod {
	CellCenter,
	QuadricErrorMetric,
	HighestWeightedVertex,
	WeightedAverage
};

struct SimplificatorResult {
	std::vector<MeshData> meshesData;

	Algorithm algorithmUsed;
	double timeTaken;

	size_t originalFaceCount;
	size_t simplifiedFaceCount;
	size_t originalVertexCount;
	size_t simplifiedVertexCount;
	size_t originalMemoryBytes;
	size_t simplifiedMemoryBytes;

	float hausdorffDistance;
	float mseError;
};

struct SimplificationOptions {
	// edge collapse validity checks
	bool checkFaceFlipping = false;
	bool checkConnectivity = false;

	bool preserveBorders = false;

	// holes and tearing prevention
	bool resolveUVSeams = false;
	bool lockUVSeams = false;
	bool enableMerging = false;
	bool mergeCloseVertivesPos = false;
	bool mergeCloseVerticesUV = false;
	bool mergeCloseVerticesNormal = false;
	
	// vdecimation parameter
	float featureAngleThreshold = 30.0f;

	// error metrics compute enablers
	bool computeHausdorff = false;
	bool computeMSE = false;
};

class Simplificator {
public:
	Simplificator();
	~Simplificator() = default;

	// enables output meshes to be flat shaded in post-processing
	void enableFlatShading(bool enableFlatShading) { flatShading = enableFlatShading; };

	// selects the algorithm to be used for simplification
	void setCurrentAlgorithm(Algorithm algorithm);

	// getters and setters
	Algorithm getCurrentAlgorithm() const { return currentAlgorithm; };
	ClusteringMethod getClusteringMethod() const { return clusteringMethod; };
	void setClusteringMethod(ClusteringMethod method) { clusteringMethod = method; };
	
	// exports the simplified meshes to an .obj file
	void exportOBJ(std::string& filename, const std::vector<MeshData>& meshData);

	/**
	* @brief Simplifies given model via per mesh simplification.
	* 
	* @param model Model to be simplified
	* @param targetFaceCountRatio Target face count ratio (0-99% - 0.01f-0.99f)
	* 
	* @return SimplificatorResult containing the simplified meshes and statistics about the simplification process
	*/
	SimplificatorResult simplify(Model& model, float targetFaceCountRatio);
	SimplificationOptions options;

private:
	Algorithm currentAlgorithm;
	ClusteringMethod clusteringMethod = ClusteringMethod::CellCenter;
	bool flatShading = false;

	/**
	* @brief Simplifies a mesh via the Quadric Error Metric using edge collapse.
	* 
	* @param vertices The vertices of the mesh to be simplified
	* @param indices The indices of the mesh to be simplified
	* @param targetFaceCount The target face count to be achieved by the simplification process
	* 
	* @return A MeshData struct containing the simplified mesh's vertices and indices
	*/
	MeshData simplifyQEM(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount);

	/**
	* @brief Simplifies a mesh via vertex decimation, which iteratively removes vertices based on the specified feature angle threshold.
	* 
	* @param vertices The vertices of the mesh to be simplified
	* @param indices The indices of the mesh to be simplified
	* @param targetFaceCount The target face count to be achieved by the simplification process
	* 
	* @return A MeshData struct containing the simplified mesh's vertices and indices
	*/
	MeshData simplifyVertexDecimation(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount);

	/**
	* @brief Simplifies a mesh via vertex clustering, which groups vertices into a grid and replaces them with representative vertices based on the specified clustering method.
	* 
	* @param vertices The vertices of the mesh to be simplified
	* @param indices The indices of the mesh to be simplified
	* @param cellsPerAxis The number of cells per longest axis to be used for clustering
	* 
	* @return A MeshData struct containing the simplified mesh's vertices and indices
	*/
	MeshData simplifyVertexClustering(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t cellsPerAxis);

	/**
	* @brief Simplifies a mesh via floating cell clustering, using the specified clustering method.
	*
	* @param vertices The vertices of the mesh to be simplified
	* @param indices The indices of the mesh to be simplified
	* @param cellsPerAxis The number of cells per longest axis to be used for clustering
	*
	* @return A MeshData struct containing the simplified mesh's vertices and indices
	*/
	MeshData simplifyFloatingCellClustering(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t cellRadius);

	/**
	* @brief Simplifies a mesh via a naive edge collapse approach, which iteratively collapses the shortest edge till the criterion is met.
	* 
	* @param vertices The vertices of the mesh to be simplified
	* @param indices The indices of the mesh to be simplified
	* @param targetFaceCount The target face count to be achieved by the simplification process
	* 
	* @return A MeshData struct containing the simplified mesh's vertices and indices
	*/
	MeshData simplifyNaive(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount);

	/**
	* @brief Simplifies a mesh via random vertex removal, which removes random edges till the criterion is met.
	* 
	* @param vertices The vertices of the mesh to be simplified
	* @param indices The indices of the mesh to be simplified
	* @param targetFaceCount The target face count to be achieved by the simplification process
	* 
	* @return A MeshData struct containing the simplified mesh's vertices and indices
	*/
	MeshData simplifyRandom(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount);
};

/* End of the Simplificator.hpp file */