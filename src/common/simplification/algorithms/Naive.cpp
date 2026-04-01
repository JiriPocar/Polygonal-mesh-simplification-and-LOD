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

	uint32_t collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const Edge& edgeToCollapse, std::vector<std::vector<uint32_t>>& twinMap, bool syncUVSeams, std::vector<bool>& vertexDeleted)
    {
        uint32_t keepIdx = edgeToCollapse.v1;
        uint32_t removeIdx = edgeToCollapse.v2;

		// if removeIdx is a seam vertex and keepIdx is not, turn the collapse around to keep the seam vertex
        if (syncUVSeams)
        {
            bool keepIsSeam = !twinMap[keepIdx].empty();
            bool removeIsSeam = !twinMap[removeIdx].empty();
            if (removeIsSeam && !keepIsSeam)
                std::swap(keepIdx, removeIdx);
        }

		// remap all indices from removeIdx to keepIdx in the index buffer
        Geometry::remapIndices(indices, removeIdx, keepIdx);
		// mark the removed vertex as deleted
        vertexDeleted[removeIdx] = true;

		// if syncing UV seams, we need to also remap all twin vertices of
        // removeIdx to the best matching twin of keepIdx and mark them as deleted
		if (syncUVSeams)
		{
			// keepCandidates = keepIdx + all its twins, from which we choose the best match for each twin of removeIdx
			std::vector<uint32_t> keepCandidates = twinMap[keepIdx];
			keepCandidates.push_back(keepIdx);

			// for each twin of removeIdx find the best matching twin of keepIdx and remap it
			for (auto twinToRemove : twinMap[removeIdx])
			{
				if (vertexDeleted[twinToRemove]) continue;

				// find the best matching twin in keepCandidates based on UV and normal
				uint32_t bestTwinKeep = keepIdx;
				float minDiff = FLT_MAX;
				for (uint32_t candidate : keepCandidates)
				{
					if (vertexDeleted[candidate]) continue;
					float uvDiff = glm::length(vertices[twinToRemove].texCoord - vertices[candidate].texCoord);
					float normDiff = glm::length(vertices[twinToRemove].normal - vertices[candidate].normal);
					if (uvDiff + normDiff < minDiff)
					{
						minDiff = uvDiff + normDiff;
						bestTwinKeep = candidate;
					}
				}

				// collapse twins
				Geometry::remapIndices(indices, twinToRemove, bestTwinKeep);
				vertexDeleted[twinToRemove] = true;

				// reconnect other twins of twinToRemove to bestTwinKeep
				for (uint32_t grandTwin : twinMap[twinToRemove])
				{
					if (grandTwin == bestTwinKeep || vertexDeleted[grandTwin]) continue;

					// add grandTwin to bestTwinKeep's twin list if not already there
					auto& btkTwins = twinMap[bestTwinKeep];
					if (std::find(btkTwins.begin(), btkTwins.end(), grandTwin) == btkTwins.end())
					{
						btkTwins.push_back(grandTwin);
					}

					// deal with grandTwin's twin list - replace twinToRemove with bestTwinKeep
					auto& gtTwins = twinMap[grandTwin];
					std::replace(gtTwins.begin(), gtTwins.end(), twinToRemove, bestTwinKeep);
				}
				twinMap[twinToRemove].clear();
			}

			// reconnect twins of removeIdx to keepIdx
			for (uint32_t twin : twinMap[removeIdx])
			{
				if (vertexDeleted[twin] || twin == keepIdx) continue;

				// add twin to keepIdx's twin list if not already there
				auto& keepIdxTwin = twinMap[keepIdx];
				if (std::find(keepIdxTwin.begin(), keepIdxTwin.end(), twin) == keepIdxTwin.end())
				{
					keepIdxTwin.push_back(twin);
				}

				// deal with twin's twin list - replace removeIdx with keepIdx
				auto& tTwins = twinMap[twin];
				std::replace(tTwins.begin(), tTwins.end(), removeIdx, keepIdx);
			}
			twinMap[removeIdx].clear();
		}

        Geometry::removeDegeneratedTriangles(indices);
        return keepIdx;
	}
}

 /* End of the Naive.cpp file */