#pragma once

#include "../resources/Model.hpp"

#define DONT_DECIMATE_ERROR 1e9

enum class Algorithm {
	QEM,
	VertexClustering,
	FloatingCellClustering,
	VertexDecimation,
	Naive
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
};

struct CollapseOptions {
	bool checkFaceFlipping = false;
	bool checkConnectivity = false;
	bool preserveBorders = false;
	bool resolveUVSeams = false;
	bool lockUVSeams = false;

	bool enableMerging = false;
	bool mergeCloseVertivesPos = false;
	bool mergeCloseVerticesUV = false;
	bool mergeCloseVerticesNormal = false;
	float featureAngleThreshold = 30.0f;
};

class Simplificator {
public:
	Simplificator();
	~Simplificator() = default;

	void enableFlatShading(bool enableFlatShading) { flatShading = enableFlatShading; };

	void setCurrentAlgorithm(Algorithm algorithm);
	Algorithm getCurrentAlgorithm() const { return currentAlgorithm; };

	void setClusteringMethod(ClusteringMethod method) { clusteringMethod = method; };
	ClusteringMethod getClusteringMethod() const { return clusteringMethod; };

	SimplificatorResult simplify(Model& model, float targetFaceCountRatio);
	CollapseOptions options;

private:
	Algorithm currentAlgorithm;
	ClusteringMethod clusteringMethod = ClusteringMethod::CellCenter;
	bool flatShading = false;

	MeshData simplifyQEM(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount);
	MeshData simplifyFloatingCellClustering(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t cellRadius);
	MeshData simplifyVertexDecimation(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount);
	MeshData simplifyVertexClustering(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t cellsPerAxis);
	MeshData simplifyNaive(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount);
};