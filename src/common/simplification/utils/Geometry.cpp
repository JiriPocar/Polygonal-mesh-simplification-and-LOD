/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Geometry.cpp
 * @brief Utility functions for mesh geometry and geometry manipulations.
 *
 * This file contains ...
 */

#include "Geometry.hpp"
#include <algorithm>
#include <numeric>

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

		for (size_t i = 0; i < indices.size(); i += 3)
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

	void mergeCloseVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, CollapseOptions& options, float threshold)
	{
		// optimization inspired by https://leanrada.com/notes/sweep-and-prune/
		// briefly explained here http://parallel.vub.ac.be/documentation/pvm/Example/Marc_Ramaekers/node3.html
		// sort vertices by x coordinate and only compare vertices that are close in x coordinate,
		// since if they are far in x coordinate, they cannot be close in 3D space

		// initially, each vertex is its own representative
		std::vector<uint32_t> indexMap(vertices.size());
		for (uint32_t i = 0; i < vertices.size(); i++)
		{
			indexMap[i] = i;
		}

		// sort vertices by x coordinate
		std::vector<uint32_t> sortedIndices(vertices.size());
		std::iota(sortedIndices.begin(), sortedIndices.end(), 0);
		std::sort(sortedIndices.begin(), sortedIndices.end(),
			[&vertices](uint32_t a, uint32_t b) {
				return vertices[a].pos.x < vertices[b].pos.x;
			}
		);

		// compare each vertex with the following vertices in the sorted order
		for (uint32_t i = 0; i < sortedIndices.size(); i++)
		{
			uint32_t idx1 = sortedIndices[i];
			if (indexMap[idx1] != idx1) continue; // already merged with another vertex

			for (uint32_t j = i + 1; j < sortedIndices.size(); j++)
			{
				uint32_t idx2 = sortedIndices[j];

				// prune - break for vertices that are way too far in x coordinate
				if (vertices[idx2].pos.x - vertices[idx1].pos.x > threshold)
				{
					break;
				}

				if (indexMap[idx2] != idx2) continue; // already merged with another vertex

				bool canMerge = true;

				if (options.mergeCloseVertivesPos)
				{
					// KEEP THIS CHECK since we are only comparing vertices that are close in x coordinate,
					// but they still may be far in y and z coordinates
					if (glm::length2(vertices[idx1].pos - vertices[idx2].pos) > threshold * threshold)
					{
						canMerge = false;
					}
				}
				else
				{
					canMerge = false;
				}

				if (canMerge && options.mergeCloseVerticesUV)
				{
					if (glm::length2(vertices[idx1].texCoord - vertices[idx2].texCoord) > 0.001f * 0.001f)
					{
						canMerge = false;
					}
				}

				if (canMerge && options.mergeCloseVerticesNormal)
				{
					if (glm::length2(vertices[idx1].normal - vertices[idx2].normal) > 0.01f * 0.01f)
					{
						canMerge = false;
					}
				}

				// in index map, set the representative of idx2 to be the representative of idx1
				if (canMerge)
				{
					indexMap[idx2] = idx1;
				}
			}
		}

		// remap indices to the representative vertex index
		for (auto& idx : indices) {
			idx = indexMap[idx];
		}

		removeDegeneratedTriangles(indices);
	}

	void makeFlatShaded(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<Vertex> flatVertices;
		std::vector<uint32_t> flatIndices;

		// for each triangle
		for (size_t i = 0; i < indices.size(); i += 3) {
			Vertex v0 = vertices[indices[i]];
			Vertex v1 = vertices[indices[i + 1]];
			Vertex v2 = vertices[indices[i + 2]];

			// compute triangle normal
			glm::vec3 normal = glm::normalize(glm::cross(v1.pos - v0.pos, v2.pos - v0.pos));
			v0.normal = normal;
			v1.normal = normal;
			v2.normal = normal;

			// set each vertex normal to the trinangle normal
			flatIndices.push_back(flatVertices.size());
			flatVertices.push_back(v0);

			flatIndices.push_back(flatVertices.size());
			flatVertices.push_back(v1);

			flatIndices.push_back(flatVertices.size());
			flatVertices.push_back(v2);
		}

		// assign back
		vertices = flatVertices;
		indices = flatIndices;
	}

	void finalizeVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::unordered_map<uint32_t, uint32_t> oldToNew;
		std::vector<Vertex> newVertices;
		std::vector<uint32_t> newIndices;
		newIndices.reserve(indices.size());

		for (uint32_t oldIdx : indices)
		{
			if (oldToNew.find(oldIdx) == oldToNew.end())
			{
				oldToNew[oldIdx] = static_cast<uint32_t>(newVertices.size());
				newVertices.push_back(vertices[oldIdx]);
			}
			newIndices.push_back(oldToNew[oldIdx]);
		}

		vertices = std::move(newVertices);
		indices = std::move(newIndices);
	}
}
 /* End of the Geometry.cpp file */