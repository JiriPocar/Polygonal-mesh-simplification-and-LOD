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

SimplificatorResult Simplificator::simplify(const Model& model, float targetFaceCountRatio)
{
	SimplificatorResult result;
	auto start = std::chrono::high_resolution_clock::now();

	auto originalFaceCount = model.extractIndices().size() / 3;
	uint32_t targetFaceCount;

	std::cout << "Simplify called with parameter: " << targetFaceCountRatio << std::endl;

	switch (currentAlgorithm) {
		case Algorithm::QEM:
			targetFaceCount = originalFaceCount * targetFaceCountRatio;
			result = simplifyQEM(model, targetFaceCount);
			break;
		case Algorithm::EdgeCollapse:
			targetFaceCount = originalFaceCount * targetFaceCountRatio;
			result = simplifyEdgeCollapse(model, targetFaceCount);
			break;
		case Algorithm::VertexClustering:
			targetFaceCount = targetFaceCountRatio;
			result = simplifyVertexClustering(model, targetFaceCount);
			break;
		case Algorithm::Naive:
			targetFaceCount = originalFaceCount * targetFaceCountRatio;
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
	auto vertices = model.extractVertices();
	auto indices = model.extractIndices();

	mergeCloseVertices(vertices, indices);
	size_t currentFaceCount = indices.size() / 3;
	size_t resultCompareFaceCount = currentFaceCount;

	std::cout << "=== QEM Simplification ===" << std::endl;
	std::cout << "Input: " << vertices.size() << " vertices, "
		<< currentFaceCount << " faces" << std::endl;
	std::cout << "Target: " << targetFaceCount << " faces" << std::endl;

	if (currentFaceCount <= targetFaceCount)
	{
		result.vertices = vertices;
		result.indices = indices;
		return result;
	}

	// init quadrics
	auto quadrics = SimplificationUtil::initQuadrics(vertices, indices);

	// create Qedges
	auto qedges = SimplificationUtil::createQedges(vertices, indices, quadrics);

	while (currentFaceCount > targetFaceCount && !qedges.empty())
	{
		auto minEdge = SimplificationUtil::findMinErr(qedges);
		SimplificationUtil::collapseQedge(vertices, indices, quadrics, minEdge);
		SimplificationUtil::updateAfterCollapse(qedges, minEdge.v2, minEdge.v1, vertices, quadrics);
		currentFaceCount = indices.size() / 3;
	}

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << resultCompareFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / resultCompareFaceCount) << "%)" << std::endl;

	result.vertices = vertices;
	result.indices = indices;
	return result;
}

SimplificatorResult Simplificator::simplifyEdgeCollapse(const Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	result.vertices = model.extractVertices();
	result.indices = model.extractIndices();
	return result;
}

SimplificatorResult Simplificator::simplifyVertexClustering(const Model& model, size_t cellsPerAxis)
{
	SimplificatorResult result;
	auto vertices = model.extractVertices();
	auto indices = model.extractIndices();

	mergeCloseVertices(vertices, indices);
	size_t currentFaceCount = indices.size() / 3;

	std::cout << "=== Vertex Clustering Debug ===" << std::endl;
	std::cout << "Input vertices: " << vertices.size() << std::endl;
	std::cout << "Input faces: " << currentFaceCount << std::endl;
	std::cout << "Cells per axis: " << cellsPerAxis << std::endl;

	float gridSize = SimplificationUtil::computeGridCellSize(vertices, cellsPerAxis);

	auto grid = SimplificationUtil::createGrid(vertices, gridSize);
	std::cout << "Grid dimensions: " << grid.sizeX << "x"
		<< grid.sizeY << "x" << grid.sizeZ << std::endl;

	SimplificationUtil::fillGrid(grid, vertices);

	auto indexMap = SimplificationUtil::computeClusterCentroids(grid, vertices);
	SimplificationUtil::remapIndices(indices, indexMap);

	SimplificationUtil::removeDegeneratedTriangles(indices);
	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << currentFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / currentFaceCount) << "%)" << std::endl;

	result.vertices = vertices;
	result.indices = indices;
	return result;
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

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << currentFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / currentFaceCount) << "%)" << std::endl;

	result.vertices = vertices;
	result.indices = indices;
	return result;
}