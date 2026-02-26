#include "Simplificator.hpp"
#include "utils/Geometry.hpp"
#include "utils/Topology.hpp"
#include "algorithms/QEM.hpp"
#include "algorithms/VertexClustering.hpp"
#include "algorithms/VertexDecimation.hpp"
#include "algorithms/Naive.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>

Simplificator::Simplificator()
{
	currentAlgorithm = Algorithm::Naive;
}

void Simplificator::setCurrentAlgorithm(Algorithm algorithm)
{
	currentAlgorithm = algorithm;
}

SimplificatorResult Simplificator::simplify(Model& model, float targetFaceCountRatio)
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
		case Algorithm::VertexClustering:
			targetFaceCount = targetFaceCountRatio;
			result = simplifyVertexClustering(model, targetFaceCount);
			break;
		case Algorithm::Naive:
			targetFaceCount = originalFaceCount * targetFaceCountRatio;
			result = simplifyNaive(model, targetFaceCount);
			break;
		case Algorithm::FloatingCellClustering:
			targetFaceCount = targetFaceCountRatio;
			result = simplifyFloatingCellClustering(model, targetFaceCount);
			break;
		case Algorithm::VertexDecimation:
			targetFaceCount = originalFaceCount * targetFaceCountRatio;
			result = simplifyVertexDecimation(model, targetFaceCount);
			break;
	}

	auto end = std::chrono::high_resolution_clock::now();
	double timeTaken = std::chrono::duration<double, std::milli>(end - start).count();

	result.algorithmUsed = currentAlgorithm;
	result.timeTaken = timeTaken;
	result.simplifiedFaceCount = result.indices.size() / 3;

	std::cout << "======== Simplification took: " << timeTaken/1000 << " seconds ======" << std::endl;

	if(flatShading)
	{
		Geometry::makeFlatShaded(result.vertices, result.indices);
	}

	return result;
}

SimplificatorResult Simplificator::simplifyQEM(Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	auto vertices = model.extractVertices();
	auto indices = model.extractIndices();

	Geometry::mergeCloseVertices(vertices, indices);
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

	std::vector<bool> isBorderVertex = Topology::findBorderVertices(vertices, indices);

	// init quadrics
	auto quadrics = QEM::initQuadrics(vertices, indices);

	// create Qedges
	auto qedges = QEM::createQedges(vertices, indices, quadrics, isBorderVertex);

	while (currentFaceCount > targetFaceCount && !qedges.empty())
	{
		QEM::Qedge minEdge;
		bool canSimplifyFurther = QEM::getValidMinEdge(qedges, vertices, indices, minEdge);
		if (!canSimplifyFurther)
		{
			std::cout << "=== No valid edge found, stopping simplification. ===" << std::endl;
			break;
		}

		QEM::collapseQedge(vertices, indices, quadrics, minEdge);
		QEM::updateAfterCollapse(qedges, minEdge.v2, minEdge.v1, vertices, quadrics, isBorderVertex);
		currentFaceCount = indices.size() / 3;
	}

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << resultCompareFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / resultCompareFaceCount) << "%)" << std::endl;

	result.vertices = vertices;
	result.indices = indices;
	return result;
}

SimplificatorResult Simplificator::simplifyFloatingCellClustering(Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	result.vertices = model.extractVertices();
	result.indices = model.extractIndices();
	return result;
}

SimplificatorResult Simplificator::simplifyVertexDecimation(Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	auto vertices = model.extractVertices();
	auto indices = model.extractIndices();

	Geometry::mergeCloseVertices(vertices, indices);
	size_t currentFaceCount = indices.size() / 3;
	size_t resultCompareFaceCount = currentFaceCount;

	std::cout << "=== Vertex Decimation ===" << std::endl;
	std::cout << "Input: " << vertices.size() << " vertices, "
		<< currentFaceCount << " faces" << std::endl;
	std::cout << "Target: " << targetFaceCount << " faces" << std::endl;

	if (currentFaceCount <= targetFaceCount)
	{
		result.vertices = vertices;
		result.indices = indices;
		return result;
	}

	auto vertexInfo = VertexDecimation::computeVertexInfo(indices, vertices.size());
	std::vector<VertexDecimation::DecimationCandidate> candidates;
	candidates.reserve(vertices.size());

	for (uint32_t i = 0; i < vertices.size(); i++)
	{
		vertexInfo[i].classification = VertexDecimation::classifyVertex(i, vertices, indices, vertexInfo[i]);
		if (vertexInfo[i].classification != VertexDecimation::VertexClassification::Complex &&
			vertexInfo[i].classification != VertexDecimation::VertexClassification::Undefined)
		{
			double err = VertexDecimation::computeVertexError(i, vertices, indices, vertexInfo[i]);
			candidates.push_back({ i, err });
		}
	}

	while (currentFaceCount > targetFaceCount && !candidates.empty())
	{
		auto minErr = std::min_element(candidates.begin(), candidates.end(),
			[](const VertexDecimation::DecimationCandidate& a, const VertexDecimation::DecimationCandidate& b) {
				return a.error < b.error;
			});

		if (minErr->error > 1e7) break;

		uint32_t vIdx = minErr->vertexIdx;

		if (!vertexInfo[vIdx].isActive)
		{
			minErr->error = 1e9;
			continue;
		}

		std::vector<uint32_t> newTriangles = VertexDecimation::triangulateHole(vIdx, vertices, indices, vertexInfo[vIdx]);
		if (newTriangles.empty())
		{
			minErr->error = 1e9;
			continue;
		}

		size_t removedFaces = vertexInfo[vIdx].neighborhood.triangles.size();
		size_t createdFaces = newTriangles.size() / 3;

		VertexDecimation::updateLocalTopology(vertexInfo, candidates, newTriangles, indices, vIdx, vertices);
		
		currentFaceCount = currentFaceCount - removedFaces + createdFaces;

		minErr->error = 1e9; // mark as processed
	}

	Geometry::removeDegeneratedTriangles(indices);

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << resultCompareFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / resultCompareFaceCount) << "%)" << std::endl;

	result.vertices = vertices;
	result.indices = indices;
	return result;
}

SimplificatorResult Simplificator::simplifyVertexClustering(Model& model, size_t cellsPerAxis)
{
	SimplificatorResult result;
	auto vertices = model.extractVertices();
	auto indices = model.extractIndices();

	//mergeCloseVertices(vertices, indices);
	size_t currentFaceCount = indices.size() / 3;

	std::cout << "=== Vertex Clustering Debug ===" << std::endl;
	std::cout << "Input vertices: " << vertices.size() << std::endl;
	std::cout << "Input faces: " << currentFaceCount << std::endl;
	std::cout << "Cells per axis: " << cellsPerAxis << std::endl;

	float gridSize = VertexClustering::computeGridCellSize(vertices, cellsPerAxis);

	auto grid = VertexClustering::createGrid(vertices, gridSize);
	std::cout << "Grid dimensions: " << grid.sizeX << "x"
		<< grid.sizeY << "x" << grid.sizeZ << std::endl;

	VertexClustering::fillGrid(grid, vertices);

	std::unordered_map<uint32_t, uint32_t> indexMap;

	switch (clusteringMethod)
	{
		case ClusteringMethod::CellCenter:
			indexMap = VertexClustering::computeRepresentativesCellCentre(grid, vertices);
			break;
		case ClusteringMethod::QuadricErrorMetric:
		{
			auto quadrics = QEM::initQuadrics(vertices, indices);
			indexMap = VertexClustering::computeRepresentativesQEM(grid, vertices, quadrics);
		}
			break;
		case ClusteringMethod::HighestWeightedVertex:
		{
			std::vector<float> vertexWeights(vertices.size(), 0.0f);

			auto allNeighborhoods = Topology::buildAllNeighborhoods(vertices.size(), indices);

			for (uint32_t i = 0; i < vertices.size(); i++)
			{
				vertexWeights[i] = VertexClustering::calculateVertexWeight(i, vertices, indices, allNeighborhoods[i]);
			}

			indexMap = VertexClustering::computeRepresentativesHighestWeight(grid, vertices, vertexWeights);
		}
			break;
		case ClusteringMethod::WeightedAverage:
		{
			std::vector<float> vertexWeights(vertices.size(), 0.0f);

			auto allNeighborhoods = Topology::buildAllNeighborhoods(vertices.size(), indices);

			for (uint32_t i = 0; i < vertices.size(); i++)
			{
				vertexWeights[i] = VertexClustering::calculateVertexWeight(i, vertices, indices, allNeighborhoods[i]);
			}

			indexMap = VertexClustering::computeRepresentativesMeanWeight(grid, vertices, vertexWeights);
		}
			break;
		default:
			break;
	}

	Geometry::remapIndices(indices, indexMap);
	Geometry::removeDegeneratedTriangles(indices);

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << currentFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / currentFaceCount) << "%)" << std::endl;

	result.vertices = vertices;
	result.indices = indices;
	return result;
}

SimplificatorResult Simplificator::simplifyNaive(Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	auto vertices = model.extractVertices();
	auto indices = model.extractIndices();

	Geometry::mergeCloseVertices(vertices, indices);
	size_t currentFaceCount = indices.size() / 3;

	while (currentFaceCount > targetFaceCount)
	{
		auto edges = Naive::getEdgesInModel(indices);
		if (edges.empty()) break;

		auto edgeToCollapse = Naive::findShortestEdge(vertices, edges);
		Naive::collapseEdge(vertices, indices, edgeToCollapse);
		currentFaceCount = indices.size() / 3;
	}

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << currentFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / currentFaceCount) << "%)" << std::endl;

	result.vertices = vertices;
	result.indices = indices;
	return result;
}