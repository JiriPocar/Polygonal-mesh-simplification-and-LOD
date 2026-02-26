/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Naive.cpp
 * @brief Naive algorithm implementation.
 *
 * This file contains ...
 */

#include "Naive.hpp"

namespace Naive {
	std::vector<Edge> getEdgesInModel(const std::vector<uint32_t>& indices)
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

	Edge findShortestEdge(const std::vector<Vertex>& vertices, const std::vector<Edge>& edges)
	{
		Edge shortestEdge;
		float minLen = FLT_MAX;

		for (auto& edge : edges)
		{
			float len = Geometry::getEdgeLength(vertices[edge.v1].pos, vertices[edge.v2].pos);
			if (len < minLen)
			{
				minLen = len;
				shortestEdge = edge;
			}
		}

		// TODO: resolve empty model
		return shortestEdge;
	}

	void collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const Edge& edgeToCollapse)
	{
		uint32_t keepIdx = edgeToCollapse.v1;
		uint32_t removeIdx = edgeToCollapse.v2;

		Geometry::remapIndices(indices, removeIdx, keepIdx);
		Geometry::removeDegeneratedTriangles(indices);
	}
}

 /* End of the Naive.cpp file */