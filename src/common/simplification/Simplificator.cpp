/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Simplificator.cpp
 * @brief Simplification implementation.
 *
 * This file implements the simplification processes used for polygonal
 * mesh simplification, including various algorithms and error metrics.
 */

#include "Simplificator.hpp"
#include "utils/Geometry.hpp"
#include "utils/Topology.hpp"
#include "utils/LazyPriorityQueue.hpp"
#include "utils/ErrorMetrics.hpp"
#include "algorithms/QEM.hpp"
#include "algorithms/VertexClustering.hpp"
#include "algorithms/FloatingCellClustering.hpp"
#include "algorithms/VertexDecimation.hpp"
#include "algorithms/Naive.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <fstream>

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
	float scale = model.getScaleIndex();

	result.originalFaceCount = 0;
	result.simplifiedFaceCount = 0;
	result.originalVertexCount = 0;
	result.simplifiedVertexCount = 0;
	result.originalMemoryBytes = 0;
	result.simplifiedMemoryBytes = 0;
	result.hausdorffDistance = 0.0f;
	result.mseError = 0.0f;

	float totalSquaredErrorSum = 0.0f;
	size_t totalVertexCount = 0;

	// per mesh simplification
	const auto& meshes = model.getMeshes();
	for (const auto& mesh : meshes)
	{
		auto vertices = mesh->extractVertices();
		auto indices = mesh->extractIndices();

		// =========================================================================
		// pre-processing
		if (options.enableMerging)
		{
			Geometry::mergeCloseVertices(vertices, indices, options);
		}

		size_t originalFaceCount = indices.size() / 3;

		size_t targetFaceCount;
		if (currentAlgorithm == Algorithm::VertexClustering || currentAlgorithm == Algorithm::FloatingCellClustering)
		{
			targetFaceCount = static_cast<size_t>(targetFaceCountRatio);
		}
		else
		{
			targetFaceCount = static_cast<size_t>(originalFaceCount * targetFaceCountRatio);

			// prevent over-simplification for small meshes in the model
			size_t minFaces = std::min<size_t>(originalFaceCount, 24);
			targetFaceCount = std::max(targetFaceCount, minFaces);
		}
		// =========================================================================

		// =========================================================================
		// simplification loop / apply clustering
		MeshData simplifiedMesh;
		switch (currentAlgorithm)
		{
			case Algorithm::QEM: simplifiedMesh = simplifyQEM(vertices, indices, targetFaceCount); break;
			case Algorithm::VertexClustering: simplifiedMesh = simplifyVertexClustering(vertices, indices, targetFaceCount); break;
			case Algorithm::Naive: simplifiedMesh = simplifyNaive(vertices, indices, targetFaceCount); break;
			case Algorithm::FloatingCellClustering: simplifiedMesh = simplifyFloatingCellClustering(vertices, indices, targetFaceCount); break;
			case Algorithm::VertexDecimation: simplifiedMesh = simplifyVertexDecimation(vertices, indices, targetFaceCount); break;
			case Algorithm::Random: simplifiedMesh = simplifyRandom(vertices, indices, targetFaceCount); break;
		}
		// =========================================================================

		// =========================================================================
		// post-processing
		Geometry::finalizeVertices(simplifiedMesh.vertices, simplifiedMesh.indices);

		if (options.computeHausdorff)
		{
			float hausdorffDistance = ErrorMetrics::getHausdorffDistance(vertices, indices, simplifiedMesh.vertices, simplifiedMesh.indices);
			
			// normalize via scaling factor
			float normalizedHausdorff = hausdorffDistance * scale;
			result.hausdorffDistance = std::max(result.hausdorffDistance, normalizedHausdorff);
		}

		if (options.computeMSE)
		{
			// have to do it like this since simplifiying each mesh separately
			float rawSquaredErrorSumAB = ErrorMetrics::computeOneSideSquaredDistance(vertices, simplifiedMesh.vertices, simplifiedMesh.indices);
			float rawSquaredErrorSumBA = ErrorMetrics::computeOneSideSquaredDistance(simplifiedMesh.vertices, vertices, indices);
			
			// normalize via scaling factor and accumulate
			totalSquaredErrorSum += (rawSquaredErrorSumAB + rawSquaredErrorSumBA) * (scale * scale);
			
			totalVertexCount += vertices.size() + simplifiedMesh.vertices.size();
		}

		if (flatShading)
		{
			Geometry::makeFlatShaded(simplifiedMesh.vertices, simplifiedMesh.indices);
		}
		else
		{
			Geometry::recalculateSmoothNormals(simplifiedMesh.vertices, simplifiedMesh.indices);
		}
		// =========================================================================

		result.originalFaceCount += originalFaceCount;
		result.simplifiedFaceCount += simplifiedMesh.indices.size() / 3;
		result.originalVertexCount += vertices.size();
		result.simplifiedVertexCount += simplifiedMesh.vertices.size();
		result.originalMemoryBytes += (vertices.size() * sizeof(Vertex)) + (indices.size() * sizeof(uint32_t));
		result.simplifiedMemoryBytes += (simplifiedMesh.vertices.size() * sizeof(Vertex)) + (simplifiedMesh.indices.size() * sizeof(uint32_t));
	
		result.meshesData.push_back(std::move(simplifiedMesh));
	}

	if (options.computeMSE && totalVertexCount > 0)
	{
		result.mseError = totalSquaredErrorSum / totalVertexCount;
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

	auto reps = Topology::buildSamePlaceRepresentatives(vertices);
	auto twinMap = Topology::buildTwinMap(reps);
	std::vector<bool> isBorderVertex = Topology::findLockedVertices(vertices, indices, reps, options);
	
	size_t currentFaceCount = Topology::countActiveFaces(indices, reps);
	size_t resultCompareFaceCount = currentFaceCount;

	std::cout << "\n=== QEM Simplification ===" << std::endl;
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
	if (options.resolveUVSeams)
	{
		for (uint32_t i = 0; i < vertices.size(); i++)
		{
			if (reps[i] != i) continue;
			if (twinMap[i].empty()) continue;

			QEM::Quadric combined = quadrics[i];
			for (uint32_t twin : twinMap[i])
			{
				combined = combined + quadrics[twin];
			}

			quadrics[i] = combined;
			for (uint32_t twin : twinMap[i])
			{
				quadrics[twin] = combined;
			}
		}
	}

	auto allNeighborhoods = Topology::buildAllNeighborhoods(vertices.size(), indices);

	// init lazy priority queue and deleted vertex tracking
	LazyPriorityQueue<QEM::Qedge, QEM::QedgeCompare> qedgeQueue;
	std::vector<bool> vertexDeleted(vertices.size(), false);

	// fill priority queue with initial edges
	auto startEdges = QEM::createQedges(vertices, indices, quadrics, isBorderVertex);
	for (auto& edge : startEdges)
	{
		qedgeQueue.push(edge);
	}

	// build a context struct to pass around necessary data
	QEM::QEMContext context{vertices, indices, quadrics, vertexDeleted, isBorderVertex, twinMap, allNeighborhoods};

	// simplification loop
	while (currentFaceCount > targetFaceCount)
	{
		// pop a valid, minimal error edge from the prio queue
		QEM::Qedge minEdge;
		bool foundValidEdge = qedgeQueue.popValid(minEdge, [&](const QEM::Qedge& e) {
			return QEM::isEdgeValidForCollapse(context, e, qedgeQueue, options);
		});

		// if no valid edge was found or the error is too high, stop the simplification
		if (!foundValidEdge)
		{
			std::cout << "Abrupt stop - no more edges to collapse that would fit the criteria." << std::endl;
			break;
		}

		if (options.resolveUVSeams)
		{
			bool keepIsSeam = !twinMap[minEdge.v1].empty();
			bool removeIsSeam = !twinMap[minEdge.v2].empty();

			if (removeIsSeam && !keepIsSeam)
			{
				std::swap(minEdge.v1, minEdge.v2);
			}
		}

		int deletedFaces = QEM::collapseQedge(context, minEdge);
		uint32_t keepIdx = minEdge.v1;
		uint32_t removeIdx = minEdge.v2;

		if (options.resolveUVSeams)
		{
			deletedFaces += QEM::syncSeamTwinsAfterCollapse(context, keepIdx, removeIdx, minEdge.optimalPos);
		}

		currentFaceCount -= deletedFaces;

		QEM::enqueueAffectedEdges(context, keepIdx, qedgeQueue, options);
	}

	Geometry::removeDegeneratedTriangles(indices);

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << resultCompareFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / resultCompareFaceCount) << "%)" << std::endl;

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

MeshData Simplificator::simplifyVertexDecimation(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount)
{
	MeshData result;
	
	size_t currentFaceCount = indices.size() / 3;
	size_t resultCompareFaceCount = currentFaceCount;

	std::cout << "\n=== Vertex Decimation ===" << std::endl;
	std::cout << "Input: " << vertices.size() << " vertices, "
		<< currentFaceCount << " faces" << std::endl;
	std::cout << "Target: " << targetFaceCount << " faces" << std::endl;

	if (currentFaceCount <= targetFaceCount)
	{
		result.vertices = vertices;
		result.indices = indices;
		return result;
	}

	auto reps = Topology::buildSamePlaceRepresentatives(vertices);
	std::vector<bool> isLockedVertex = Topology::findLockedVertices(vertices, indices, reps, options);

	auto allNeighborhoods = Topology::buildAllNeighborhoods(vertices.size(), indices);
	std::vector<VertexDecimation::VertexInfo> vInfo(vertices.size());
	for (uint32_t i = 0; i < vertices.size(); i++)
	{
		vInfo[i].neighborhood = allNeighborhoods[i];
		vInfo[i].classification = VertexDecimation::VertexClassification::Undefined;
		vInfo[i].isActive = true;
	}

	// init priority queue
	LazyPriorityQueue<VertexDecimation::DecimationCandidate, VertexDecimation::DecimationCompare> candidatesQueue;

	// fill with initial candidates
	for (uint32_t i = 0; i < vertices.size(); i++)
	{
		vInfo[i].classification = VertexDecimation::classifyVertex(i, vertices, indices, vInfo[i], options);
		if (vInfo[i].classification != VertexDecimation::VertexClassification::Complex && vInfo[i].classification != VertexDecimation::VertexClassification::Undefined)
		{
			double err = VertexDecimation::computeVertexError(i, vertices, indices, vInfo[i], options, isLockedVertex);
			candidatesQueue.push({ i, err });
		}
	}

	// simplification loop
	while (currentFaceCount > targetFaceCount)
	{
		// pop a valid, minimal error candidate from the prio queue
		VertexDecimation::DecimationCandidate minErrCandidate;
		bool foundValid = candidatesQueue.popValid(minErrCandidate, [&](const VertexDecimation::DecimationCandidate& c) {
			// skip inactive
			if (!vInfo[c.vertexIdx].isActive) return false;

			// compare stored error with actual error to detect outdated candidates
			double actualError = VertexDecimation::computeVertexError(c.vertexIdx, vertices, indices, vInfo[c.vertexIdx], options, isLockedVertex);
			if (std::abs(c.error - actualError) > 1e-5)
			{
				return false;
			}

			return true;
		});

		// if not another valid candidate was found or the error is too high, stop the decimation
		if (!foundValid || minErrCandidate.error > DONT_DECIMATE_ERROR)
		{
			std::cout << "Abrupt stop - no more valid candidates for decimation." << std::endl;
			break;
		}

		// retriangulate the hole left by vertex removal
		uint32_t vIdx = minErrCandidate.vertexIdx;
		std::vector<uint32_t> newTriangles = VertexDecimation::triangulateHole(vIdx, vertices, indices, vInfo[vIdx]);

		// if triangulation failed, mark vertex as complex to avoid trying to delete it again
		if (newTriangles.empty())
		{
			vInfo[vIdx].classification = VertexDecimation::VertexClassification::Complex;
			continue;
		}

		// every triangle of the removed vertex is removed
		size_t deletedFaces = vInfo[vIdx].neighborhood.triangles.size();

		// every triangle in the triangulated hole is added
		size_t addedFaces = newTriangles.size() / 3;

		// readd to lazy pqueue and update topology
		VertexDecimation::updateLocalTopology(vInfo, candidatesQueue, newTriangles, indices, vIdx, vertices, options, isLockedVertex);

		currentFaceCount = currentFaceCount - deletedFaces + addedFaces;
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

	size_t currentFaceCount = indices.size() / 3;

	std::cout << "\n=== Vertex Clustering ===" << std::endl;
	std::cout << "Input vertices: " << vertices.size() << std::endl;
	std::cout << "Input faces: " << currentFaceCount << std::endl;
	std::cout << "Cells per axis: " << cellsPerAxis << std::endl;

	// get cell size based on longest axis of the models bounding box
	float gridSize = VertexClustering::computeGridCellSize(vertices, cellsPerAxis);

	// create the 3D grid and fill it with vertex indices
	auto grid = VertexClustering::createGrid(vertices, gridSize);
	std::cout << "Grid dimensions: " << grid.sizeX << "x" << grid.sizeY << "x" << grid.sizeZ << std::endl;
	VertexClustering::fillGrid(grid, vertices);

	// map for remapping indices
	std::unordered_map<uint32_t, uint32_t> indexMap;

	// pick a representative vertex strategy
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

	// remap indices and remove degenerated triangles
	Geometry::remapIndices(indices, indexMap);
	Geometry::removeDegeneratedTriangles(indices);

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << currentFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / currentFaceCount) << "%)" << std::endl;

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

MeshData Simplificator::simplifyFloatingCellClustering(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t cellRadius)
{
	MeshData result;

	size_t currentFaceCount = indices.size() / 3;

	std::cout << "\n=== Floating-Cell Clustering ===" << std::endl;
	std::cout << "Input vertices: " << vertices.size() << std::endl;
	std::cout << "Input faces: " << currentFaceCount << std::endl;
	std::cout << "Cell radius: " << cellRadius << std::endl;

	// radius is computed based on the user defined cells per axis parameter
	float radius = FloatingCellClustering::computeRadius(vertices, cellRadius);
	auto indexMap = FloatingCellClustering::computeRepresentative(vertices, indices, radius);

	// remap indices and remove degenerated triangles
	Geometry::remapIndices(indices, indexMap);
	Geometry::removeDegeneratedTriangles(indices);

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

MeshData Simplificator::simplifyNaive(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount)
{
	MeshData result;

	auto reps = Topology::buildSamePlaceRepresentatives(vertices);
	auto twinMap = Topology::buildTwinMap(reps);
	std::vector<bool> isBorderVertex = Topology::findLockedVertices(vertices, indices, reps, options);

	size_t currentFaceCount = indices.size() / 3;
	size_t resultFaceCount = currentFaceCount;

	std::cout << "\n=== Naive shortest edge collapse ===" << std::endl;
	std::cout << "Input: " << vertices.size() << " vertices, "
		<< currentFaceCount << " faces" << std::endl;
	std::cout << "Target: " << targetFaceCount << " faces" << std::endl;

	auto allNeighborhoods = Topology::buildAllNeighborhoods(vertices.size(), indices);

	LazyPriorityQueue<Naive::Edge, Naive::EdgeCompare> edgeQueue;
	std::vector<bool> vertexDeleted(vertices.size(), false);

	auto startEdges = Naive::getEdgesInModel(indices);
	for (auto& edge : startEdges)
	{
		edge.length = glm::length(vertices[edge.v1].pos - vertices[edge.v2].pos);
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

		if (!Naive::isCollapseValid(shortestEdge.v1, shortestEdge.v2, vertices, indices, options, isBorderVertex, allNeighborhoods))
		{
			continue;
		}
		
		int deletedFaces = Naive::collapseEdge(vertices, indices, shortestEdge, vertexDeleted, allNeighborhoods);
		currentFaceCount -= deletedFaces;
		uint32_t keepIdx = shortestEdge.v1;

		for (uint32_t tri : allNeighborhoods[keepIdx].triangles)
		{
			uint32_t i0 = indices[tri];
			uint32_t i1 = indices[tri + 1];
			uint32_t i2 = indices[tri + 2];

			if (i0 == i1 || i1 == i2 || i2 == i0) continue;

			Naive::Edge e1 = { std::min(i0, i1), std::max(i0, i1), glm::distance(vertices[i0].pos, vertices[i1].pos) };
			Naive::Edge e2 = { std::min(i1, i2), std::max(i1, i2), glm::distance(vertices[i1].pos, vertices[i2].pos) };
			Naive::Edge e3 = { std::min(i2, i0), std::max(i2, i0), glm::distance(vertices[i2].pos, vertices[i0].pos) };

			edgeQueue.push(e1);
			edgeQueue.push(e2);
			edgeQueue.push(e3);
		}
	}

	Geometry::removeDegeneratedTriangles(indices);

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << resultFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / resultFaceCount) << "%)" << std::endl;

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

MeshData Simplificator::simplifyRandom(std::vector<Vertex> vertices, std::vector<uint32_t> indices, size_t targetFaceCount)
{
	MeshData result;

	auto allNeighborhoods = Topology::buildAllNeighborhoods(vertices.size(), indices);
	std::vector<uint32_t> vertexDeleted(vertices.size(), false);

	size_t currentFaceCount = indices.size() / 3;
	size_t originalFaceCount = currentFaceCount;

	while (currentFaceCount > targetFaceCount)
	{
		// pick random vertex
		uint32_t randomVertexIdx = rand() % vertices.size();
		if (vertexDeleted[randomVertexIdx])
		{
			continue;
		}

		// get triangles of the random vertex, if it has none, skip
		auto& triangles = allNeighborhoods[randomVertexIdx].triangles;
		if (triangles.empty())
		{
			continue;
		}

		// pick a random triangle connected with the random vertex 
		uint32_t randomTri = triangles[rand() % triangles.size()];
		uint32_t v0 = indices[randomTri];
		uint32_t v1 = indices[randomTri + 1];
		uint32_t v2 = indices[randomTri + 2];

		uint32_t keepIdx;
		if (randomVertexIdx == v0)
		{
			keepIdx = (rand() % 2 == 0) ? v1 : v2;
		}
		else if (randomVertexIdx == v1)
		{
			keepIdx = (rand() % 2 == 0) ? v0 : v2;
		}
		else
		{
			keepIdx = (rand() % 2 == 0) ? v0 : v1;
		}

		if (options.checkFaceFlipping)
		{
			if (Topology::checkFaceFlipping(vertices[randomVertexIdx].pos, vertices[keepIdx].pos, randomVertexIdx, indices, vertices, allNeighborhoods[randomVertexIdx]))
			{
				continue;
			}
		}

		if (options.checkConnectivity)
		{
			if (!Topology::checkConnectivity(randomVertexIdx, keepIdx, indices, allNeighborhoods[randomVertexIdx], allNeighborhoods[keepIdx]))
			{
				continue;
			}
		}

		// update all triangles of the removed vertex to point to the kept vertex
		for (uint32_t tri : triangles)
		{
			uint32_t idx0 = indices[tri];
			uint32_t idx1 = indices[tri + 1];
			uint32_t idx2 = indices[tri + 2];

			// skip degenerate triangles
			if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0) continue;

			// if triangle contains the kept vertex, it will degenerate due to collapse
			bool willDegenerate = (idx0 == keepIdx || idx1 == keepIdx || idx2 == keepIdx);
			if (willDegenerate) currentFaceCount--;

			// redirect triangle to kept vertex
			if (indices[tri] == randomVertexIdx) indices[tri] = keepIdx;
			if (indices[tri + 1] == randomVertexIdx) indices[tri + 1] = keepIdx;
			if (indices[tri + 2] == randomVertexIdx) indices[tri + 2] = keepIdx;

			// if triangle didnt degenerate, add it to the neighborhood of the kept vertex
			if (!willDegenerate)
			{
				allNeighborhoods[keepIdx].triangles.push_back(tri);
			}
		}

		// clear triangles of the removed vertex
		triangles.clear();

		// after redirecting all triangles, some triangles in the neighborhood of the kept vertex might have become degenerate
		auto& keepTris = allNeighborhoods[keepIdx].triangles;
		keepTris.erase(std::remove_if(keepTris.begin(), keepTris.end(),
			[&indices](uint32_t tri) {
				return indices[tri] == indices[tri + 1] ||
					indices[tri + 1] == indices[tri + 2] ||
					indices[tri + 2] == indices[tri];
			}),
			keepTris.end());

		vertexDeleted[randomVertexIdx] = true;
	}

	Geometry::removeDegeneratedTriangles(indices);

	size_t finalFaceCount = indices.size() / 3;
	std::cout << "Final faces: " << finalFaceCount << std::endl;
	std::cout << "Reduction: " << originalFaceCount << " -> " << finalFaceCount << " (" << (100.0f * finalFaceCount / originalFaceCount) << "%)" << std::endl;

	result.vertices = std::move(vertices);
	result.indices = std::move(indices);
	return result;
}

void Simplificator::exportOBJ(std::string& filename, const std::vector<MeshData>& meshData)
{
	std::ofstream objFile(filename);

	// .obj indexing starts at 1
	uint32_t vertexOffset = 1;
	for (const auto& mesh : meshData)
	{
		// write vertices
		for (const auto& v : mesh.vertices)
		{
			objFile << "v " << v.pos.x << " " << v.pos.y << " " << v.pos.z << "\n";
		}

		// write triangles
		for (uint32_t i = 0; i < mesh.indices.size(); i += 3)
		{
			objFile << "f" << " " << (mesh.indices[i]     + vertexOffset) << " "
								  << (mesh.indices[i + 1] + vertexOffset) << " "
								  << (mesh.indices[i + 2] + vertexOffset) << "\n";
		}

		// update vertex offset for the next mesh
		vertexOffset += mesh.vertices.size();
	}

	objFile.close();
	std::cout << "Exported simplified model to " << filename << std::endl;
}

/* End of the Simplificator.cpp file */