#include "simplificationUtil.hpp"
#include <algorithm>
#include <iostream>

namespace SimplificationUtil {

	float getEdgeLength(const glm::vec3& v1, const glm::vec3& v2)
	{
		return glm::length(v2 - v1);
	}

	void remapIndices(std::vector<uint32_t>& indices, uint32_t oldIdx, uint32_t newIdx)
	{
		for (auto& index : indices) {
			if (index == oldIdx) {
				index = newIdx;
			}
		}
	}

	std::vector<Edge> getEdgesInModel(std::vector<uint32_t>& indices)
	{
		std::vector<Edge> resultEdges;

		// for each triplet of indices (each triangle)
		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t triplet[3] = { indices[i], indices[i + 1], indices[i + 2] };

			// for each edge in the triangle
			for (int j = 0; j < 3; j++)
			{
				Edge e;
				e.v1 = std::min(triplet[j], triplet[(j + 1) % 3]);
				e.v2 = std::max(triplet[j], triplet[(j + 1) % 3]);

				// check if edge was already added
				bool exists = false;
				for (auto& existingEdge : resultEdges)
				{
					if (existingEdge.v1 == e.v1 && existingEdge.v2 == e.v2)
					{
						exists = true;
						break;
					}
				}

				// if not, add it
				if (!exists)
				{
					resultEdges.push_back(e);
				}
			}
		}

		return resultEdges;
	}

	Edge findShortestEdge(std::vector<Vertex>& vertices, std::vector<Edge>& edges)
	{
		Edge shortestEdge;
		float minLen = FLT_MAX;

		for (auto& edge : edges)
		{
			float len = getEdgeLength(vertices[edge.v1].pos, vertices[edge.v2].pos);
			if (len < minLen)
			{
				minLen = len;
				shortestEdge = edge;
			}
		}

		// TODO: resolve empty model
		return shortestEdge;
	}

	void removeDegeneratedTriangles(std::vector<uint32_t>& indices)
	{
		std::vector<uint32_t> cleanedIndices;

		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t i1 = indices[i];
			uint32_t i2 = indices[i + 1];
			uint32_t i3 = indices[i + 2];

			if (i1 != i2 && i2 != i3 && i1 != i3)
			{
				cleanedIndices.push_back(i1);
				cleanedIndices.push_back(i2);
				cleanedIndices.push_back(i3);
			}
		}
		indices = cleanedIndices;
	}


	void collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Edge& edgeToCollapse)
	{
		uint32_t keepIdx = edgeToCollapse.v1;
		uint32_t removeIdx = edgeToCollapse.v2;

		remapIndices(indices, removeIdx, keepIdx);
		removeDegeneratedTriangles(indices);

	}
}