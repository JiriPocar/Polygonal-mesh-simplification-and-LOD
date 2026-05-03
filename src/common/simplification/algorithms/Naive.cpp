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
			// can use squared length for comparison to avoid unnecessary sqrt
			float len = glm::length2(vertices[edge.v1].pos - vertices[edge.v2].pos);
			if (len < minLen)
			{
				minLen = len;
				shortestEdge = edge;
			}
		}

		// TODO: resolve empty model
		return shortestEdge;
	}

	int collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const Edge& edgeToCollapse, std::vector<bool>& vertexDeleted, std::vector<Topology::Neighborhood>& allNeighborhoods)
    {
        uint32_t keepIdx = edgeToCollapse.v1;
        uint32_t removeIdx = edgeToCollapse.v2;
        int outDeletedFaces = 0;

        if (keepIdx == removeIdx) return keepIdx;

        // update all triangles in the neighborhood of the removed vertex
        for (uint32_t tri : allNeighborhoods[removeIdx].triangles)
        {
            uint32_t idx0 = indices[tri];
            uint32_t idx1 = indices[tri + 1];
            uint32_t idx2 = indices[tri + 2];

            // skip degenerate triangles
            if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0)
            {
                continue;
            }

            // if we replace removeIdx with keepIdx in this triangle, will it become degenerate?
            bool willDegenerate = (idx0 == keepIdx || idx1 == keepIdx || idx2 == keepIdx);
            if (willDegenerate) outDeletedFaces++;

            // set removeIdx to keepIdx in this triangle
            // HALF EDGE COLLAPSE
            if (indices[tri] == removeIdx) indices[tri] = keepIdx;
            if (indices[tri + 1] == removeIdx) indices[tri + 1] = keepIdx;
            if (indices[tri + 2] == removeIdx) indices[tri + 2] = keepIdx;

            // if not degenerate, add this removed vertex triangle to the neighborhood of the kept vertex
            if (!willDegenerate)
            {
                allNeighborhoods[keepIdx].triangles.push_back(tri);
            }
        }

        // clear triangle neighborhood of the removed vertex
        allNeighborhoods[removeIdx].triangles.clear();

        // remove degenerate triangles from the neighborhood of the kept vertex
        auto& tris = allNeighborhoods[keepIdx].triangles;
        tris.erase(std::remove_if(tris.begin(), tris.end(),
            [&indices](uint32_t tri) {
                return indices[tri] == indices[tri + 1] ||
                    indices[tri + 1] == indices[tri + 2] ||
                    indices[tri + 2] == indices[tri];
            }),
            tris.end());

		// mark removed vertex as deleted
        vertexDeleted[removeIdx] = true;
        return outDeletedFaces;
    }

    bool isCollapseValid(uint32_t v1, uint32_t v2, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, SimplificationOptions& options, const std::vector<bool>& isBorderVertex, std::vector<Topology::Neighborhood>& allNeighborhoods)
    {
        if (options.preserveBorders)
        {
            if (isBorderVertex[v1] || isBorderVertex[v2])
            {
                return false;
            }
        }

        if (options.lockUVSeams)
        {
            if (isBorderVertex[v1] || isBorderVertex[v2])
            {
                return false;
            }
        }

        if (options.checkFaceFlipping)
        {
            if (Topology::checkFaceFlipping(vertices[v2].pos, vertices[v1].pos, v2, indices, vertices, allNeighborhoods[v1]))
            {
                return false;
            }
        }

        if (options.checkConnectivity)
        {
            if (!checkConnectivity(v1, v2, indices, allNeighborhoods[v1], allNeighborhoods[v2]))
            {
                return false;
            }
        }

        return true;
    }
}

 /* End of the Naive.cpp file */