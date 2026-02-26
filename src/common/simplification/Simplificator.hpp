#pragma once

#include "../resources/Model.hpp"

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
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Algorithm algorithmUsed;
	double timeTaken;

	size_t originalFaceCount;
	size_t simplifiedFaceCount;
	size_t originalVertexCount;
	size_t simplifiedVertexCount;
	size_t originalMemoryBytes;
	size_t simplifiedMemoryBytes;
};

class Simplificator {
public:
	Simplificator();
	~Simplificator() = default;

	void setCurrentAlgorithm(Algorithm algorithm);
	void enableFlatShading(bool enableFlatShading) { flatShading = enableFlatShading; };
	Algorithm getCurrentAlgorithm() const { return currentAlgorithm; };

	void setClusteringMethod(ClusteringMethod method) { clusteringMethod = method; };
	ClusteringMethod getClusteringMethod() const { return clusteringMethod; };

	SimplificatorResult simplify(Model& model, float targetFaceCountRatio);

private:
	Algorithm currentAlgorithm;
	ClusteringMethod clusteringMethod = ClusteringMethod::CellCenter;
	bool flatShading;

	SimplificatorResult simplifyQEM(Model& model, size_t targetFaceCount);
	SimplificatorResult simplifyFloatingCellClustering(Model& model, size_t cellRadius);
	SimplificatorResult simplifyVertexDecimation(Model& model, size_t targetFaceCount);
	SimplificatorResult simplifyVertexClustering(Model& model, size_t cellsPerAxis);
	SimplificatorResult simplifyNaive(Model& model, size_t targetFaceCount);
};