/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file FloatingCellClustering.cpp
 * @brief Floating cell clustering algorithm implementation.
 *
 * This file contains ...
 */

 /* End of the FloatingCellCLustering.cpp file */

#include "FloatingCellClustering.hpp"
#include <algorithm>
#include <numeric>

#include "../utils/Topology.hpp"
#include "../utils/Geometry.hpp"
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
		std::unordered_map<uint32_t, uint32_t> indexRemap;
		size_t vertexCount = vertices.size();
		
		// build a weight for each vertex based on its neighborhood
		std::vector<float> weights(vertexCount);
		std::vector<uint32_t> sortedIndices(vertexCount);
		auto neighborhoods = Topology::buildAllNeighborhoods(vertexCount, indices);

		for (uint32_t i = 0; i < vertexCount; i++)
		{
			weights[i] = VertexClustering::calculateVertexWeight(i, vertices, indices, neighborhoods[i]);
			sortedIndices[i] = i;
		}

		// sort vertices by weight in descending order
		std::sort(sortedIndices.begin(), sortedIndices.end(),
			[&weights](uint32_t x, uint32_t y)
			{
				return weights[x] > weights[y];
			}
		);

		// flagging process
		std::vector<bool> isFlagged(vertexCount, false);
		for (uint32_t centerIdx : sortedIndices)
		{
			// skip if vertex is already flagged by another center
			if (isFlagged[centerIdx]) continue;

			isFlagged[centerIdx] = true;
			glm::vec3 centerPos = vertices[centerIdx].pos;

			// flag all vertices within radius and remap them to the center vertex
			for (uint32_t i = 0; i < vertexCount; i++)
			{
				// skip if vertex is already flagged by another center
				if (isFlagged[i]) continue;

				if (glm::distance(centerPos, vertices[i].pos) <= radius)
				{
					isFlagged[i] = true;
					indexRemap[i] = centerIdx;
				}
			}
		}

		return indexRemap;
	}
}