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
		// optimization inspired by https://leanrada.com/notes/sweep-and-prune/
		// briefly explained here http://parallel.vub.ac.be/documentation/pvm/Example/Marc_Ramaekers/node3.html
		// sort vertices by x coordinate and only compare vertices that are close in x coordinate,
		// since if they are far in x coordinate, they cannot be close in 3D space

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

	size_t countActiveFaces(const std::vector<uint32_t>& indices, const std::vector<uint32_t>& reps)
	{
		size_t count = 0;
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			// if the representatives of all three vertices of this triangle are different, it is an active face
			uint32_t r0 = reps[indices[i]];
			uint32_t r1 = reps[indices[i + 1]];
			uint32_t r2 = reps[indices[i + 2]];

			if (r0 != r1 && r1 != r2 && r0 != r2)
			{
				count++;
			}
		}
		return count;
	}

	std::vector<bool> findLockedVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<uint32_t>& representatives, const SimplificationOptions& options)
	{
		std::vector<bool> isLocked(vertices.size(), false);

		// if preserving borders, lock all vertices that are connected by an edge that belongs to only one triangle
		if (options.preserveBorders)
		{
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
						isLocked[rawV1] = true;
						isLocked[rawV2] = true;
					}
				}
			}
		}

		if (options.lockUVSeams)
		{
			std::vector<int> repCount(vertices.size(), 0);
			for (uint32_t i = 0; i < vertices.size(); i++)
			{
				repCount[representatives[i]]++;
			}

			for (uint32_t i = 0; i < vertices.size(); i++)
			{
				if (repCount[representatives[i]] > 1)
				{
					isLocked[i] = true;
				}
			}
		}

		return isLocked;
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
			// meshoptimizer by Arseny Kapoulkine uses similar threshold of 0.2
			if (glm::dot(normOld, normNew) < 0.2f)
			{
				return true;
			}
		}

		return false;
	}

	bool checkConnectivity(uint32_t v1, uint32_t v2, std::vector<uint32_t>& indices, const Neighborhood& n1, const Neighborhood& n2)
	{
		// count shared faces
		int sharedFaces = 0;
		for (uint32_t t1 : n1.triangles)
		{
			uint32_t idx0 = indices[t1];
			uint32_t idx1 = indices[t1 + 1];
			uint32_t idx2 = indices[t1 + 2];

			if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0)
			{
				continue;
			}

			for (uint32_t t2 : n2.triangles)
			{
				if (t1 == t2)
				{
					sharedFaces++;
					break;
				}
			}
		}

		// count unique v1 vertices
		std::vector<uint32_t> v1Vertices;
		for (uint32_t t : n1.triangles)
		{
			uint32_t idx0 = indices[t];
			uint32_t idx1 = indices[t + 1];
			uint32_t idx2 = indices[t + 2];
			
			if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0)
			{
				continue;
			}

			if (idx0 != v1) v1Vertices.push_back(idx0);
			if (idx1 != v1) v1Vertices.push_back(idx1);
			if (idx2 != v1) v1Vertices.push_back(idx2);
		}
		std::sort(v1Vertices.begin(), v1Vertices.end());
		v1Vertices.erase(std::unique(v1Vertices.begin(), v1Vertices.end()), v1Vertices.end());

		// count unique v2 faces
		std::vector<uint32_t> v2Vertices;
		for (uint32_t t : n2.triangles)
		{
			uint32_t idx0 = indices[t];
			uint32_t idx1 = indices[t + 1];
			uint32_t idx2 = indices[t + 2];

			if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0)
			{
				continue;
			}

			if (idx0 != v2) v2Vertices.push_back(idx0);
			if (idx1 != v2) v2Vertices.push_back(idx1);
			if (idx2 != v2) v2Vertices.push_back(idx2);
		}
		std::sort(v2Vertices.begin(), v2Vertices.end());
		v2Vertices.erase(std::unique(v2Vertices.begin(), v2Vertices.end()), v2Vertices.end());

		// count shared vertices
		int sharedVertices = 0;
		for (uint32_t v1v : v1Vertices)
		{
			for (uint32_t v2v : v2Vertices)
			{
				if (v1v == v2v)
				{
					sharedVertices++;
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

}
/* End of the Topology.cpp file */