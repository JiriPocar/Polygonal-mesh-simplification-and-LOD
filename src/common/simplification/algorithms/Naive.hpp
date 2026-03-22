/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Naive.hpp
 * @brief Naive algorithm implementation.
 *
 * This file contains ...
 */

#pragma once
#include <vector>
#include <algorithm>
#include "../../resources/Model.hpp"
#include "../utils/Geometry.hpp"

namespace Naive {

    struct Edge {
        uint32_t v1, v2;
        float length;

        bool isIdentical(uint32_t e1, uint32_t e2) const
        {
            uint32_t minE = std::min(e1, e2);
            uint32_t maxE = std::max(e1, e2);
            return (v1 == minE && v2 == maxE);
        }
    };

	// comparator for priority queue to sort edges by length (shortest first)
    struct EdgeCompare {
        bool operator()(const Edge& a, const Edge& b) const
        {
            // larger length is lower priority
			return a.length > b.length;
        }
    };

    std::vector<Edge> getEdgesInModel(const std::vector<uint32_t>& indices);
    Edge findShortestEdge(const std::vector<Vertex>& vertices, const std::vector<Edge>& edges);
    uint32_t collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const Edge& edgeToCollapse, std::vector<std::vector<uint32_t>>& twinMap, bool syncUVSeams, std::vector<bool>& vertexDeleted);

}

 /* End of the Naive.hpp file */