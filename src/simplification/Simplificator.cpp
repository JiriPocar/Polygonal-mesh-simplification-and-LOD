#include "Simplificator.hpp"
#include "simplificationUtil.hpp"
#include <chrono>
#include <iostream>

Simplificator::Simplificator()
{
	currentAlgorithm = Algorithm::Naive;
}

void Simplificator::setCurrentAlgorithm(Algorithm algorithm)
{
	currentAlgorithm = algorithm;
}

SimplificatorResult Simplificator::simplify(const Model& model, float targetFaceCountRatio)
{
	SimplificatorResult result;
	auto start = std::chrono::high_resolution_clock::now();

	auto originalFaceCount = model.extractIndices().size() / 3;
	uint32_t targetFaceCount = originalFaceCount * targetFaceCountRatio;
	switch (currentAlgorithm) {
		case Algorithm::QEM:
			result = simplifyQEM(model, targetFaceCount);
			break;
		case Algorithm::EdgeCollapse:
			result = simplifyEdgeCollapse(model, targetFaceCount);
			break;
		case Algorithm::VertexClustering:
			result = simplifyVertexClustering(model, targetFaceCount);
			break;
		case Algorithm::Naive:
			result = simplifyNaive(model, targetFaceCount);
			break;
	}

	auto end = std::chrono::high_resolution_clock::now();
	double timeTaken = std::chrono::duration<double, std::milli>(end - start).count();

	result.algorithmUsed = currentAlgorithm;
	result.timeTaken = timeTaken;
	result.simplifiedFaceCount = result.indices.size() / 3;

	return result;
}

SimplificatorResult Simplificator::simplifyQEM(const Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	result.vertices = model.extractVertices();
	result.indices = model.extractIndices();
	return result;
}

SimplificatorResult Simplificator::simplifyEdgeCollapse(const Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	result.vertices = model.extractVertices();
	result.indices = model.extractIndices();
	return result;
}

SimplificatorResult Simplificator::simplifyVertexClustering(const Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	result.vertices = model.extractVertices();
	result.indices = model.extractIndices();
	return result;
}

void mergeCloseVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float threshold = 0.0001f)
{
	// build index map, each vertex points to its representative
	// if vertex is duplicate, then map to the first occurrence
	std::vector<uint32_t> indexMap(vertices.size());

	for (uint32_t i = 0; i < vertices.size(); i++) {
		indexMap[i] = i;

		// find duplicate vertex
		for (uint32_t j = 0; j < i; j++) {
			if (glm::length(vertices[i].pos - vertices[j].pos) < threshold) {
				indexMap[i] = indexMap[j]; // rewrite as duplicate
				break;
			}
		}
	}

	// remap indexes
	for (auto& idx : indices) {
		idx = indexMap[idx];
	}

	// remove degenerated triangles
	SimplificationUtil::removeDegeneratedTriangles(indices);
}

SimplificatorResult Simplificator::simplifyNaive(const Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	auto vertices = model.extractVertices();
	auto indices = model.extractIndices();

	mergeCloseVertices(vertices, indices);
	size_t currentFaceCount = indices.size() / 3;

	while (currentFaceCount > targetFaceCount)
	{
		auto edges = SimplificationUtil::getEdgesInModel(indices);
		if (edges.empty()) break;

		auto edgeToCollapse = SimplificationUtil::findShortestEdge(vertices, edges);
		SimplificationUtil::collapseEdge(vertices, indices, edgeToCollapse);
		currentFaceCount = indices.size() / 3;
	}

	result.vertices = vertices;
	result.indices = indices;
	return result;
}