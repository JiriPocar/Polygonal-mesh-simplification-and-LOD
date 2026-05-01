/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file FloatingCellClustering.cpp
 * @brief Floating cell clustering algorithm implementation.
 *
 * This file contains implementation of Vertex clustering modification called
 * Floating cell clustering, which is based on the original vertex clustering
 * algorithm, but uses floating cells instead of fixed 3D grid.
 * 
 * =======================================================================================
 * 
 * Inspirations and sources:
 *		- The very original Vertex clustering algorithm definition
 *		    - "Multi-resolution 3D approximation for rendering complex scenes" by Jarek Rossignac and Paul Borrel
 *              - @url https://www.researchgate.net/publication/225075920_Multi-resolution_3D_approximation_for_rendering_complex_scenes
 *		- Floating cell clustering modification
 *			- "Model Simplification Using Vertex-Clustering" by Kok-Lim Low and Tiow-Seng Tan
 *              - @url https://www.comp.nus.edu.sg/~tants/Paper/simplify.pdf
 *		- Optimization
 *			- Sweep and prune for finding close vertices in 3D space
 *				- @url https://leanrada.com/notes/sweep-and-prune/
 * 
 * =======================================================================================
 */

#include "FloatingCellClustering.hpp"
#include <algorithm>
#include <numeric>

#include "common/simplification/utils/Topology.hpp"
#include "common/simplification/utils/Geometry.hpp"
#include "VertexClustering.hpp"

namespace FloatingCellClustering {

	float computeRadius(std::vector<Vertex>& vertices, size_t cellsPerAxis)
	{
		// alg input is number of cells per the longest axis
		// compute the bounding
		glm::vec3 minBounds;
		glm::vec3 maxBounds;
		Geometry::computeBounds(vertices, minBounds, maxBounds);

		// find the longest axis
		glm::vec3 size = maxBounds - minBounds;
		float maxAxisLen = std::max({ size.x, size.y, size.z });

		// radius is half of the cell size
		return (maxAxisLen / (float)cellsPerAxis) * 0.5f;
	}

	std::unordered_map<uint32_t, uint32_t> computeRepresentative(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius)
	{
		// optimization inspired by https://leanrada.com/notes/sweep-and-prune/
		// briefly explained here http://parallel.vub.ac.be/documentation/pvm/Example/Marc_Ramaekers/node3.html
		// sort vertices by x coordinate and only compare vertices that are close in x coordinate,
		// since if they are far in x coordinate, they cannot be close in 3D space

		std::unordered_map<uint32_t, uint32_t> indexRemap;
		size_t vertexCount = vertices.size();
		
		// build a weight for each vertex based on its neighborhood
		std::vector<float> weights(vertexCount);
		std::vector<uint32_t> sortedWeigthIndices(vertexCount);
		auto neighborhoods = Topology::buildAllNeighborhoods(vertexCount, indices);

		for (uint32_t i = 0; i < vertexCount; i++)
		{
			weights[i] = VertexClustering::calculateVertexWeight(i, vertices, indices, neighborhoods[i]);
			sortedWeigthIndices[i] = i;
		}

		// sort vertices by weight in descending order
		std::sort(sortedWeigthIndices.begin(), sortedWeigthIndices.end(),
			[&weights](uint32_t x, uint32_t y)
			{
				return weights[x] > weights[y];
			}
		);

		// sort vertices by x coordinate
		std::vector<uint32_t> sortedXIndices(vertexCount);
		std::iota(sortedXIndices.begin(), sortedXIndices.end(), 0);
		std::sort(sortedXIndices.begin(), sortedXIndices.end(),
			[&vertices](uint32_t a, uint32_t b) {
				return vertices[a].pos.x < vertices[b].pos.x;
			}
		);

		// build a map from vertex index to its position in the sorted x
		std::vector<uint32_t> posInSortedX(vertexCount);
		for (uint32_t i = 0; i < vertexCount; i++)
		{
			posInSortedX[sortedXIndices[i]] = i;
		}

		// flagging process
		std::vector<bool> isFlagged(vertexCount, false);
		for (uint32_t centerIdx : sortedWeigthIndices)
		{
			// skip if vertex is already flagged by another center
			if (isFlagged[centerIdx]) continue;

			isFlagged[centerIdx] = true;
			glm::vec3 centerPos = vertices[centerIdx].pos;

			// jump to center vertex in the sorted x order
			uint32_t centerXPos = posInSortedX[centerIdx];

			// check right side
			for (size_t i = centerXPos + 1; i < vertexCount; i++)
			{
				uint32_t targetIdx = sortedXIndices[i];

				// break if too far in x coordinate
				if (vertices[targetIdx].pos.x - centerPos.x > radius) break;

				if (isFlagged[targetIdx]) continue;

				// use length2 instead of length to avoid sqrt for performance
				if (glm::length2(vertices[targetIdx].pos - centerPos) <= radius * radius)
				{
					isFlagged[targetIdx] = true;
					indexRemap[targetIdx] = centerIdx;
				}
			}

			// check left side
			// int cast is needed to prevent underflow
			for (int i = static_cast<int>(centerXPos) - 1; i >= 0; i--)
			{
				uint32_t targetIdx = sortedXIndices[i];

				// break if too far in x coordinate
				if (centerPos.x - vertices[targetIdx].pos.x > radius) break;

				if (isFlagged[targetIdx]) continue;

				// use length2 instead of length to avoid sqrt for performance
				if (glm::length2(vertices[targetIdx].pos - centerPos) <= radius * radius)
				{
					isFlagged[targetIdx] = true;
					indexRemap[targetIdx] = centerIdx;
				}
			}
		}

		return indexRemap;
	}
}

/* End of the FloatingCellCLustering.cpp file */