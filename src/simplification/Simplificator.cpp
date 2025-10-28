#include "Simplificator.hpp"
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

SimplificatorResult Simplificator::simplify(const Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	auto start = std::chrono::high_resolution_clock::now();

	result.originalFaceCount = model.getIndexCount() / 3;

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

SimplificatorResult Simplificator::simplifyNaive(const Model& model, size_t targetFaceCount)
{
	SimplificatorResult result;
	auto vertices = model.extractVertices();
	auto indices = model.extractIndices();

	size_t currentFaceCount = indices.size() / 3;

	// skip every second triangle (THERE ARE GOING TO BE HOLES), test of functionality only
	if (currentFaceCount > targetFaceCount) {
		std::vector<uint32_t> newIndices;
		for (size_t i = 0; i < indices.size(); i += 6) {
			if (i + 2 < indices.size()) {
				newIndices.push_back(indices[i]);
				newIndices.push_back(indices[i + 1]);
				newIndices.push_back(indices[i + 2]);
			}
		}
		indices = newIndices;
	}

	std::cout << "Naive simplification: " << currentFaceCount << " -> "
		<< (indices.size() / 3) << " faces" << std::endl;

	result.vertices = vertices;
	result.indices = indices;
	return result;
}