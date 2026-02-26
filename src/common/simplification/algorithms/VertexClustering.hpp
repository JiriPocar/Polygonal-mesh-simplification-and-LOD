/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VertexClustering.hpp
 * @brief Vertex clustering algorithm implementation.
 *
 * This file contains ...
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../../resources/Model.hpp"
#include "../utils/Topology.hpp"
#include "../utils/Geometry.hpp"
#include "QEM.hpp"

namespace VertexClustering {

    struct ClusterGrid {
        std::vector<std::vector<std::vector<std::vector<uint32_t>>>> cells;
        glm::vec3 minBounds;
        glm::vec3 maxBounds;

        float cellSize;
        int sizeX, sizeY, sizeZ;
    };

	// grid creation and filling
    float computeGridCellSize(std::vector<Vertex>& vertices, size_t cellsPerAxis);
    ClusterGrid createGrid(std::vector<Vertex>& vertices, float cellSize);
    void fillGrid(ClusterGrid& grid, std::vector<Vertex>& vertices);

    // grading
    float calculateVertexWeight(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Topology::Neighborhood& neighborhood);

    // Strategy for representative selection
    std::unordered_map<uint32_t, uint32_t> computeRepresentativesCellCentre(ClusterGrid& grid, std::vector<Vertex>& vertices);
    std::unordered_map<uint32_t, uint32_t> computeRepresentativesQEM(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<QEM::Quadric>& quadrics);
    std::unordered_map<uint32_t, uint32_t> computeRepresentativesHighestWeight(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<float>& vertexWeights);
    std::unordered_map<uint32_t, uint32_t> computeRepresentativesMeanWeight(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<float>& vertexWeights);

}

 /* End of the VertexClustering.hpp file */