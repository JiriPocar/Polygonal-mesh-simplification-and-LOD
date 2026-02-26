/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VertexDecimation.hpp
 * @brief Vertex decimation algorithm implementation.
 *
 * This file contains ...
 */

#pragma once
#include <vector>
#include "../../resources/Model.hpp"
#include "../utils/Topology.hpp"

namespace VertexDecimation {

    enum class VertexClassification {
        Simple,
        Complex,
        Boundary,
        InteriorEdge,
        Corner,
        Undefined
    };

    struct VertexInfo {
        Topology::Neighborhood neighborhood;
        VertexClassification classification = VertexClassification::Undefined;
        bool isActive = true;
    };

    struct DecimationCandidate {
        uint32_t vertexIdx;
        double error;

        bool operator>(const DecimationCandidate& other) const {
            return error > other.error;
        }
    };

    double computeVertexError(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info);
    VertexClassification classifyVertex(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info);

    std::vector<VertexInfo> computeVertexInfo(std::vector<uint32_t>& indices, size_t vertexCount);
    std::vector<uint32_t> triangulateHole(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info);

    void updateLocalTopology(std::vector<VertexInfo>& vertexInfo, std::vector<DecimationCandidate>& candidates, std::vector<uint32_t>& newTriangles, std::vector<uint32_t>& indices, uint32_t removedVertexIdx, std::vector<Vertex>& vertices);

}

 /* End of the VertexDecimation.hpp file */