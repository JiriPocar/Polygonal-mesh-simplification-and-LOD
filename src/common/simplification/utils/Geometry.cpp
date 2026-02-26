/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Geometry.cpp
 * @brief Utility functions for mesh geometry and geometry manipulations.
 *
 * This file contains ...
 */

#include "Geometry.hpp"

namespace Geometry
{
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

	void remapIndices(std::vector<uint32_t>& indices, std::unordered_map<uint32_t, uint32_t>& indexMap)
	{
		for (auto& idx : indices)
		{
			auto it = indexMap.find(idx);
			if (it != indexMap.end())
			{
				idx = it->second;
			}
		}
	}

	void computeBounds(std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax)
	{
		outMin = glm::vec3(FLT_MAX);
		outMax = glm::vec3(-FLT_MAX);

		for (auto& v : vertices)
		{
			outMin = glm::min(outMin, v.pos);
			outMax = glm::max(outMax, v.pos);
		}
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

	void mergeCloseVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float threshold)
	{
		// build index map, each vertex points to its representative
		// if vertex is duplicate, then map to the first occurrence
		std::vector<uint32_t> indexMap(vertices.size());

		for (uint32_t i = 0; i < vertices.size(); i++) {
			indexMap[i] = i;

			// find duplicate vertex
			for (uint32_t j = 0; j < i; j++) {
				bool samePos = glm::length(vertices[i].pos - vertices[j].pos) < threshold;
				if (!samePos) continue;

				bool sameUV = glm::length(vertices[i].texCoord - vertices[j].texCoord) < 0.001f;

				if (samePos && sameUV)
				{
					indexMap[i] = indexMap[j];
					break;
				}
			}
		}

		// remap indexes
		for (auto& idx : indices) {
			idx = indexMap[idx];
		}

		// remove degenerated triangles
		removeDegeneratedTriangles(indices);
	}
}
 /* End of the Geometry.cpp file */