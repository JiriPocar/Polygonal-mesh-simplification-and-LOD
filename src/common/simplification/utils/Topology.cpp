/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Topology.cpp
 * @brief Utility functions for mesh topology tracking.
 *
 * This file contains ...
 */

#include "Topology.hpp"
#include <map>
#include <numeric>

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

		// sort and remove duplicates from neighborhood.vertices
		std::sort(neighborhood.vertices.begin(), neighborhood.vertices.end());
		neighborhood.vertices.erase(std::unique(neighborhood.vertices.begin(), neighborhood.vertices.end()), neighborhood.vertices.end());

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

	std::vector<uint32_t> buildSamePlaceRepresentatives(const std::vector<Vertex>& vertices)
	{
		std::vector<uint32_t> representatives(vertices.size());

		// sort vertices by x coordinate to speed up the search for vertices in the same place
		std::vector<uint32_t> sortedIndices(vertices.size());
		std::iota(sortedIndices.begin(), sortedIndices.end(), 0);
		std::sort(sortedIndices.begin(), sortedIndices.end(),
			[&vertices](uint32_t a, uint32_t b)
			{
				return vertices[a].pos.x < vertices[b].pos.x;
			}
		);

		// initially, each vertex is its own representative
		for (uint32_t i = 0; i < vertices.size(); i++)
		{
			representatives[i] = i;
		}

		// for each vertex, find other vertices in the same place and set their representative to the same index
		for (size_t i = 0; i < sortedIndices.size(); i++)
		{
			uint32_t idx1 = sortedIndices[i];

			if (representatives[idx1] != idx1) continue;

			for (size_t j = i + 1; j < sortedIndices.size(); j++)
			{
				uint32_t idx2 = sortedIndices[j];

				if (vertices[idx2].pos.x - vertices[idx1].pos.x > 0.0001f)
				{
					break;
				}

				if (glm::length(vertices[idx1].pos - vertices[idx2].pos) < 0.0001f)
				{
					representatives[idx2] = idx1;
				}
			}
		}

		return representatives;
	}

	std::vector<std::vector<uint32_t>> buildTwinMap(const std::vector<uint32_t>& representatives)
	{
		/**
		 * Vertices 0, 3, 7 share the same representative (0)
		 * 
		 * Result:
		 * twinMap[0] = { 3, 7 }
		 * twinMap[3] = { 0, 7 }
		 * twinMap[7] = { 0, 3 }
		 * 
		 * All other vertices: twinMap[i] = {}
		*/
		std::vector<std::vector<uint32_t>> twinMap(representatives.size());

		// group vertices by their representative
		std::unordered_map<uint32_t, std::vector<uint32_t>> representativesGroups;

		// for each vertex, add it to the group of its representative
		for (uint32_t i = 0; i < representatives.size(); i++)
		{
			representativesGroups[representatives[i]].push_back(i);
		}

		for (uint32_t i = 0; i < representatives.size(); i++)
		{
			uint32_t rep = representatives[i];
			for (auto twin : representativesGroups[rep])
			{
				if (twin != i)
				{
					twinMap[i].push_back(twin);
				}
			}
		}
	
		return twinMap;
	}


	std::vector<bool> findBorderVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<uint32_t>& representatives)
	{
		std::vector<bool> isBorderVertex(vertices.size(), false);
		std::map<std::pair<uint32_t, uint32_t>, int> edgeCount;
		
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			for (int j = 0; j < 3; j++)
			{
				uint32_t r1 = representatives[indices[i + j]];
				uint32_t r2 = representatives[indices[i + (j + 1) % 3]];

				if (r1 == r2) continue;

				std::pair<uint32_t, uint32_t> edge = std::minmax(r1, r2);
				edgeCount[edge]++;
			}
		}

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			for (int j = 0; j < 3; j++)
			{
				uint32_t rawV1 = indices[i + j];
				uint32_t rawV2 = indices[i + (j + 1) % 3];

				uint32_t r1 = representatives[rawV1];
				uint32_t r2 = representatives[rawV2];

				if (r1 == r2) continue;

				if (edgeCount[std::minmax(r1, r2)] == 1)
				{
					isBorderVertex[rawV1] = true;
					isBorderVertex[rawV2] = true;
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

	bool checkConnectivity(uint32_t v1, uint32_t v2, std::vector<uint32_t>& indices)
	{
		// get neighborhoods of both vertices
		auto n1 = getVertexNeighborhood(v1, indices);
		auto n2 = getVertexNeighborhood(v2, indices);

		// get number of shared vertices
		int sharedVertices = 0;
		for (uint32_t a : n1.vertices)
		{
			for (uint32_t b : n2.vertices)
			{
				if (a == b)
				{
					sharedVertices++;
					break;
				}
			}
		}

		// get number of shared faces
		int sharedFaces = 0;
		for (uint32_t a : n1.triangles)
		{
			for (uint32_t b : n2.triangles)
			{
				if (a == b)
				{
					sharedFaces++;
					break;
				}
			}
		}

		// link condition check
		// interior edge -> 2 == 2 -> okay
		// boundary edge -> 1 == 1 -> okay
		// non-manifold edge -> 2 != 1 -> not okay
		return sharedVertices == sharedFaces;
	}

	bool isCollapseValid(uint32_t v1, uint32_t v2,std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, CollapseOptions& options, const std::vector<bool>& isBorderVertex, const std::vector<std::vector<uint32_t>>& twinMap)
	{
		if (options.preserveBorders)
		{
			if (isBorderVertex[v1] || isBorderVertex[v2])
			{
				return false;
			}
		}

		if (options.checkFaceFlipping)
		{
			if (checkFaceFlipping(vertices[v2].pos, vertices[v1].pos, v2, indices, vertices))
			{
				return false;
			}
		}

		if (options.checkConnectivity)
		{
			if (checkConnectivity(v1, v2, indices))
			{
				return false;
			}

			if (options.resolveUVSeams)
			{
				std::vector<uint32_t> keepCandidates = twinMap[v1];
				keepCandidates.push_back(v1);

				for (uint32_t twinToRemove : twinMap[v2])
				{
					uint32_t bestTwinKeep = v1;
					float minDiff = FLT_MAX;

					for (uint32_t candidate : keepCandidates)
					{
						float uvDiff = glm::length(vertices[twinToRemove].texCoord - vertices[candidate].texCoord);
						float normDiff = glm::length(vertices[twinToRemove].normal - vertices[candidate].normal);
						if (uvDiff + normDiff < minDiff)
						{
							minDiff = uvDiff + normDiff;
							bestTwinKeep = candidate;
						}
					}

					if (!checkConnectivity(bestTwinKeep, twinToRemove, indices))
					{
						return false;
					}
				}

			}
		}

		return true;
	}

	bool isCollapseValidQEM(uint32_t v1, uint32_t v2, glm::vec3 optimalPos,
		std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
		CollapseOptions& options, const std::vector<bool>& isBorderVertex)
	{
		// preserve edges that only share one face
		if (options.preserveBorders)
		{
			if (isBorderVertex[v1] || isBorderVertex[v2])
			{
				return false;
			}
		}

		if (options.checkFaceFlipping)
		{
			// check flip for both (v1, optimalPos) and (v2, optimalPos), if either flips, the collapse is invalid
			if (checkFaceFlipping(vertices[v1].pos, optimalPos, v1, indices, vertices))
			{
				return false;
			}
			if (checkFaceFlipping(vertices[v2].pos, optimalPos, v2, indices, vertices))
			{
				return false;
			}
		}

		if (options.checkConnectivity)
		{

		}

		return true;
	}
}
/* End of the Topology.cpp file */