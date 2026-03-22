#include "Simplificator.hpp"
#include "utils/Geometry.hpp"
#include "utils/Topology.hpp"
#include "utils/LazyPriorityQueue.hpp"
#include "algorithms/QEM.hpp"
#include "algorithms/VertexClustering.hpp"
#include "algorithms/FloatingCellClustering.hpp"
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

	result.originalFaceCount = 0;
	result.simplifiedFaceCount = 0;
	result.originalVertexCount = 0;
	result.simplifiedVertexCount = 0;
	result.originalMemoryBytes = 0;
	result.simplifiedMemoryBytes = 0;

	std::cout << "Simplify called with parameter: " << targetFaceCountRatio << std::endl;

	const auto& meshes = model.getMeshes();
	for (const auto& mesh : meshes)
	{
		auto vertices = mesh->extractVertices();
		auto indices = mesh->extractIndices();

		size_t originalFaceCount = indices.size() / 3;

		size_t targetFaceCount;
		if (currentAlgorithm == Algorithm::VertexClustering || currentAlgorithm == Algorithm::FloatingCellClustering)
		{
			targetFaceCount = static_cast<size_t>(targetFaceCountRatio);
		}
		else
		{
			targetFaceCount = static_cast<size_t>(originalFaceCount * targetFaceCountRatio);
		}

		MeshData simplifiedMesh;
		switch (currentAlgorithm)
		{
		case Algorithm::QEM: simplifiedMesh = simplifyQEM(vertices, indices, targetFaceCount); break;
		case Algorithm::VertexClustering: simplifiedMesh = simplifyVertexClustering(vertices, indices, targetFaceCount); break;
		case Algorithm::Naive: simplifiedMesh = simplifyNaive(vertices, indices, targetFaceCount); break;
		case Algorithm::FloatingCellClustering: simplifiedMesh = simplifyFloatingCellClustering(vertices, indices, targetFaceCount); break;
		case Algorithm::VertexDecimation: simplifiedMesh = simplifyVertexDecimation(vertices, indices, targetFaceCount); break;
		}

		Geometry::finalizeVertices(simplifiedMesh.vertices, simplifiedMesh.indices);
		if (flatShading)
		{
			Geometry::makeFlatShaded(simplifiedMesh.vertices, simplifiedMesh.indices);
		}

		result.originalFaceCount += originalFaceCount;
		result.simplifiedFaceCount += simplifiedMesh.indices.size() / 3;
		result.originalVertexCount += vertices.size();
		result.simplifiedVertexCount += simplifiedMesh.vertices.size();
		result.originalMemoryBytes += (vertices.size() * sizeof(Vertex)) + (indices.size() * sizeof(uint32_t));
		result.simplifiedMemoryBytes += (simplifiedMesh.vertices.size() * sizeof(Vertex)) + (simplifiedMesh.indices.size() * sizeof(uint32_t));
	
		result.meshesData.push_back(std::move(simplifiedMesh));
	}

	auto end = std::chrono::high_resolution_clock::now();
	double timeTaken = std::chrono::duration<double, std::milli>(end - start).count();
	result.timeTaken = timeTaken;
	result.algorithmUsed = currentAlgorithm;

	return result;
}

MeshData Simplificator::simplifyQEM(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount)
{
	MeshData result;

	if (options.enableMerging)
	{
		Geometry::mergeCloseVertices(vertices, indices, options);
	}

	auto reps = Topology::buildSamePlaceRepresentatives(vertices);
	auto twinMap = Topology::buildTwinMap(reps);
	std::vector<bool> isBorderVertex(vertices.size(), false);
	if (options.preserveBorders)
	{
		isBorderVertex = Topology::findBorderVertices(vertices, indices, reps);
	}
	
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
	auto quadrics = QEM::initQuadrics(vertices, indices);

	LazyPriorityQueue<QEM::Qedge, QEM::QedgeCompare> qedgeQueue;
	std::vector<bool> vertexDeleted(vertices.size(), false);
	auto startEdges = QEM::createQedges(vertices, indices, quadrics, isBorderVertex);
	for (auto& edge : startEdges)
	{
		qedgeQueue.push(edge);
	}

	while (currentFaceCount > targetFaceCount)
	{
		QEM::Qedge minEdge;
		
		QEM::collapseQedge(vertices, indices, quadrics, minEdge, twinMap, options.resolveUVSeams, vertexDeleted);

		currentFaceCount = indices.size() / 3;
	}

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << resultCompareFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / resultCompareFaceCount) << "%)" << std::endl;

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

MeshData Simplificator::simplifyFloatingCellClustering(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t cellRadius)
{
	MeshData result;

	size_t currentFaceCount = indices.size() / 3;

	std::cout << "=== Floating-Cell Clustering ===" << std::endl;
	std::cout << "Input vertices: " << vertices.size() << std::endl;
	std::cout << "Input faces: " << currentFaceCount << std::endl;
	std::cout << "Cell radius: " << cellRadius << std::endl;

	float radius = FloatingCellClustering::computeRadius(vertices, cellRadius);
	auto indexMap = FloatingCellClustering::computeRepresentative(vertices, indices, radius);

	Geometry::remapIndices(indices, indexMap);
	Geometry::removeDegeneratedTriangles(indices);

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

MeshData Simplificator::simplifyVertexDecimation(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount)
{
	MeshData result;

	if (options.enableMerging)
	{
		Geometry::mergeCloseVertices(vertices, indices, options);
	}
	
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

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

MeshData Simplificator::simplifyVertexClustering(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t cellsPerAxis)
{
	MeshData result;

	Geometry::mergeCloseVertices(vertices, indices);
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

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

MeshData Simplificator::simplifyNaive(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount)
{
	MeshData result;

	if (options.enableMerging)
	{
		Geometry::mergeCloseVertices(vertices, indices, options);
	}

	auto reps = Topology::buildSamePlaceRepresentatives(vertices);
	auto twinMap = Topology::buildTwinMap(reps);
	std::vector<bool> isBorderVertex(vertices.size(), false);
	if (options.preserveBorders)
	{
		isBorderVertex = Topology::findBorderVertices(vertices, indices, reps);
	}

	size_t currentFaceCount = indices.size() / 3;

	LazyPriorityQueue<Naive::Edge, Naive::EdgeCompare> edgeQueue;
	std::vector<bool> vertexDeleted(vertices.size(), false);

	auto startEdges = Naive::getEdgesInModel(indices);
	for (auto& edge : startEdges)
	{
		edge.length = Geometry::getEdgeLength(vertices[edge.v1].pos, vertices[edge.v2].pos);
		edgeQueue.push(edge);
	}

	while (currentFaceCount > targetFaceCount)
	{
		Naive::Edge shortestEdge;
		bool foundValidEdge = edgeQueue.popValid(shortestEdge, [&](const Naive::Edge& e) {
			return !vertexDeleted[e.v1] && !vertexDeleted[e.v2];
			});
		
		if (!foundValidEdge)
		{
			break;
		}

		if (!Topology::isCollapseValid(shortestEdge.v1, shortestEdge.v2, vertices, indices, options, isBorderVertex, twinMap))
		{
			continue;
		}
		
		std::vector<uint32_t> keptTwinVertices;
		if (options.resolveUVSeams)
		{
			for (auto twin : twinMap[shortestEdge.v1])
			{
				if (!vertexDeleted[twin])
				{
					keptTwinVertices.push_back(twin);
				}
			}
		}

		auto actualKeepidx = Naive::collapseEdge(vertices, indices, shortestEdge, twinMap, options.resolveUVSeams, vertexDeleted);

		std::vector<uint32_t> affectedVertices = { actualKeepidx };
		if (options.resolveUVSeams)
		{
			for (uint32_t twin : twinMap[actualKeepidx])
			{
				if (!vertexDeleted[twin])
				{
					affectedVertices.push_back(twin);
				}
			}
		}

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			uint32_t i0 = indices[i];
			uint32_t i1 = indices[i + 1];
			uint32_t i2 = indices[i + 2];

			bool isAffected = false;
			for (uint32_t aff : affectedVertices)
			{
				if (i0 == aff || i1 == aff || i2 == aff)
				{
					isAffected = true;
					break;
				}
			}

			if (isAffected)
			{
				Naive::Edge e1 = { std::min(i0, i1), std::max(i0, i1), 0.0f };
				Naive::Edge e2 = { std::min(i1, i2), std::max(i1, i2), 0.0f };
				Naive::Edge e3 = { std::min(i2, i0), std::max(i2, i0), 0.0f };

				e1.length = Geometry::getEdgeLength(vertices[e1.v1].pos, vertices[e1.v2].pos);
				e2.length = Geometry::getEdgeLength(vertices[e2.v1].pos, vertices[e2.v2].pos);
				e3.length = Geometry::getEdgeLength(vertices[e3.v1].pos, vertices[e3.v2].pos);

				edgeQueue.push(e1);
				edgeQueue.push(e2);
				edgeQueue.push(e3);
			}
		}

		currentFaceCount = indices.size() / 3;
	}

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << currentFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / currentFaceCount) << "%)" << std::endl;

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}