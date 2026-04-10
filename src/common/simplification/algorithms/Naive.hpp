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
#include "../utils/Topology.hpp"

namespace Naive {

    struct Edge {
        uint32_t v1, v2;
        float length;
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
    uint32_t collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const Edge& edgeToCollapse, std::vector<std::vector<uint32_t>>& twinMap, bool syncUVSeams, std::vector<bool>& vertexDeleted, std::vector<Topology::Neighborhood>& allNeighborhoods, int& outDeletedFaces);
    bool isCollapseValid(uint32_t v1, uint32_t v2, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, SimplificationOptions& options, const std::vector<bool>& isBorderVertex, const std::vector<std::vector<uint32_t>>& twinMap, std::vector<Topology::Neighborhood>& allNeighborhoods);
}

 /* End of the Naive.hpp file */