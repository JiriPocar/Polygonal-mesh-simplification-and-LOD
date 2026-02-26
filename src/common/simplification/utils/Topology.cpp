/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Topology.cpp
 * @brief Utility functions for mesh topology tracking.
 *
 * This file contains ...
 */

#include "Topology.hpp"

namespace Topology
{
	Topology::Neighborhood getVertexNeighborhood(uint32_t vertexIdx, std::vector<uint32_t>& indices)
	{
		Topology::Neighborhood neighborhood;

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			uint32_t i0 = indices[i];
			uint32_t i1 = indices[i + 1];
			uint32_t i2 = indices[i + 2];

			if (i0 == vertexIdx || i1 == vertexIdx || i2 == vertexIdx)
			{
				neighborhood.triangles.push_back(i);

				if (i0 != vertexIdx) neighborhood.vertices.push_back(i0);
				if (i1 != vertexIdx) neighborhood.vertices.push_back(i1);
				if (i2 != vertexIdx) neighborhood.vertices.push_back(i2);
			}
		}

		return neighborhood;
	}

	std::vector<Topology::Neighborhood> buildAllNeighborhoods(size_t vertexCount, std::vector<uint32_t>& indices)
	{
		std::vector<Topology::Neighborhood> neighborhoods(vertexCount);

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			uint32_t i0 = indices[i];
			uint32_t i1 = indices[i + 1];
			uint32_t i2 = indices[i + 2];

			neighborhoods[i0].triangles.push_back(i);
			neighborhoods[i0].vertices.push_back(i1);
			neighborhoods[i0].vertices.push_back(i2);

			neighborhoods[i1].triangles.push_back(i);
			neighborhoods[i1].vertices.push_back(i0);
			neighborhoods[i1].vertices.push_back(i2);

			neighborhoods[i2].triangles.push_back(i);
			neighborhoods[i2].vertices.push_back(i0);
			neighborhoods[i2].vertices.push_back(i1);
		}

		for (auto& n : neighborhoods) {
			std::sort(n.vertices.begin(), n.vertices.end());
			n.vertices.erase(std::unique(n.vertices.begin(), n.vertices.end()), n.vertices.end());
		}

		return neighborhoods;
	}

	std::vector<bool> findBorderVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<bool> isBorderVertex(vertices.size(), false);

		// { (v1, v2), number of faces sharing this edge }
		std::map<std::pair<uint32_t, uint32_t>, int> edgeCount;

		// fill edge count map
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			for (int j = 0; j < 3; j++)
			{
				uint32_t v1 = indices[i + j];
				uint32_t v2 = indices[i + (j + 1) % 3];

				std::pair<uint32_t, uint32_t> edge = std::minmax(v1, v2);

				// at given edge, increment count of faces sharing it
				edgeCount[edge]++;
			}
		}

		// if edge is only shared by one face, then both vertices are border vertices
		for (auto& pair : edgeCount)
		{
			if (pair.second == 1)
			{
				uint32_t v1 = pair.first.first;
				uint32_t v2 = pair.first.second;

				isBorderVertex[v1] = true;
				isBorderVertex[v2] = true;
			}
		}

		std::vector<uint32_t> uniqueActiveVertices;
		std::vector<bool> isActive(vertices.size(), false);

		for (uint32_t idx : indices) {
			if (!isActive[idx]) {
				isActive[idx] = true;
				uniqueActiveVertices.push_back(idx);
			}
		}

		// sort active vertices by X to prevent unnecessary length checks later
		std::sort(uniqueActiveVertices.begin(), uniqueActiveVertices.end(),
			[&vertices](uint32_t a, uint32_t b)
			{
				return vertices[a].pos.x < vertices[b].pos.x;
			}
		);

		// tag vertices that are very close to each other as border vertices to prevent
		// collapsing them into one (which would cause holes in the mesh)
		// this resolves non-watertight meshes made from more separate meshes better
		for (size_t i = 0; i < uniqueActiveVertices.size(); i++)
		{
			uint32_t v1 = uniqueActiveVertices[i];

			for (size_t j = i + 1; j < uniqueActiveVertices.size(); j++)
			{
				uint32_t v2 = uniqueActiveVertices[j];

				// since we did sorting by X axis, we can break early
				if (std::abs(vertices[v1].pos.x - vertices[v2].pos.x) > 0.0001f)
				{
					break;
				}

				// tag vertices that are very close to each other as border vertices
				if (glm::length(vertices[v1].pos - vertices[v2].pos) < 0.0001f)
				{
					isBorderVertex[v1] = true;
					isBorderVertex[v2] = true;
				}
			}
		}

		return isBorderVertex;
	}

	bool checkFaceFlipping(glm::vec3 beforePos, glm::vec3 afterPos, uint32_t movingVertexIdx, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices)
	{
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			uint32_t idx1 = indices[i];
			uint32_t idx2 = indices[i + 1];
			uint32_t idx3 = indices[i + 2];

			// only triangles that share the moving vertex can potentially flip, skip others
			if (idx1 != movingVertexIdx && idx2 != movingVertexIdx && idx3 != movingVertexIdx)
			{
				continue;
			}

			// pos before collapse
			glm::vec3 pos1 = (idx1 == movingVertexIdx) ? beforePos : vertices[idx1].pos;
			glm::vec3 pos2 = (idx2 == movingVertexIdx) ? beforePos : vertices[idx2].pos;
			glm::vec3 pos3 = (idx3 == movingVertexIdx) ? beforePos : vertices[idx3].pos;

			glm::vec3 d1 = pos2 - pos1;
			glm::vec3 d2 = pos3 - pos1;
			glm::vec3 normOld = glm::cross(d1, d2);

			// ignore degenerated triangles
			if (glm::length(normOld) < 1e-6f)
			{
				continue;
			}

			normOld = glm::normalize(normOld);

			// pos after collapse
			if (idx1 == movingVertexIdx) pos1 = afterPos;
			else if (idx2 == movingVertexIdx) pos2 = afterPos;
			else if (idx3 == movingVertexIdx) pos3 = afterPos;

			d1 = pos2 - pos1;
			d2 = pos3 - pos1;
			glm::vec3 normNew = glm::cross(d1, d2);

			if (glm::length(normNew) < 1e-6f)
			{
				continue;
			}

			normNew = glm::normalize(normNew);

			// if normal flips by more than ~78 degrees, consider it a face flip
			// both meshoptimizer by Arseny Kapoulkine
			if (glm::dot(normOld, normNew) < 0.2f)
			{
				return true;
			}
		}

		return false;
	}
}
/* End of the Topology.cpp file */