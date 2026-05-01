/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VertexClustering.hpp
 * @brief Vertex clustering algorithm implementation.
 *
 * This file contains the definition of the vertex clustering algorithm, including
 * the structure of the cluster grid, functions for creating and filling the grid,
 * grading vertices based on their neighborhood, and different strategies for selecting
 * representative vertices for each cluster.
 * 
 * =======================================================================================
 * 
 * Inspirations and sources:
 *      - The very original Vertex clustering algorithm definition
 *		    - "Multi-resolution 3D approximation for rendering complex scenes" by Jarek Rossignac and Paul Borrel
 *              - @url https://www.researchgate.net/publication/225075920_Multi-resolution_3D_approximation_for_rendering_complex_scenes
 *      - Vertex weight calculation
 *          - "Model Simplification Using Vertex-Clustering" by Kok-Lim Low and Tiow-Seng Tan
 *              - @url https://www.comp.nus.edu.sg/~tants/Paper/simplify.pdf
 *      - Modification using QEM for representative selection
 *          - "Out-of-Core Simplification of Large Polygonal Models" by Peter Lindstrom
 *              - @url http://www-evasion.imag.fr/Membres/Franck.Hetroy/Teaching/Geo3D/Articles/lindstrom2000.pdf
 *
 * =======================================================================================
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "common/resources/Model.hpp"
#include "common/simplification/utils/Topology.hpp"
#include "common/simplification/utils/Geometry.hpp"
#include "QEM.hpp"

namespace VertexClustering {

	// 3D grid of cells with vertex indices, along with grid bounds and cell size
    struct ClusterGrid {
        std::vector<std::vector<std::vector<std::vector<uint32_t>>>> cells;
        glm::vec3 minBounds;
        glm::vec3 maxBounds;

        float cellSize;
        int sizeX, sizeY, sizeZ;
    };

	/**
    * @brief Computes the size of the grid based on user-defined cells per axis parameter.
    * 
	* @param vertices The vertices of the mesh to be simplified
	* @param cellsPerAxis The number of cells along the longest axis of the model bounding box
    * 
    * @return cell size for the grid
    */
    float computeGridCellSize(std::vector<Vertex>& vertices, size_t cellsPerAxis);

    /**
    * @brief Initializes a 3D grid based on cellsize and bounding box of the model
    * 
	* @param vertices The vertices of the mesh to be simplified
	* @param cellSize The size of each cell in the grid
    * 
	* @return initialized ClusterGrid structure with allocated cells
    */
    ClusterGrid createGrid(std::vector<Vertex>& vertices, float cellSize);

    /**
    * @brief Fills the grid with corresponding vertex indices based on their positions.
    * 
	* @param grid The ClusterGrid structure to be filled with vertex indices
	* @param vertices The vertices of the mesh to be simplified
    */
    void fillGrid(ClusterGrid& grid, std::vector<Vertex>& vertices);

    /**
    * @brief Calculates weight of a vertex at vertexIdx based on its neighborhood.
    * 
	* @param vertexIdx The index of the vertex being evaluated for weight calculation
	* @param vertices The vertices of the mesh to be simplified
	* @param indices The triangle indices of the mesh to be simplified
	* @param neighborhood The neighborhood structure containing adjacent vertices and triangles
    * 
	* @return weight of vertex at vertexIdx
    */
    float calculateVertexWeight(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Topology::Neighborhood& neighborhood);

    /**
    * @brief Strategies to select representative vertex for each cell of the uniform 3D grid.
    * 
	* @param grid The ClusterGrid structure containing the cells with vertex indices
	* @param vertices The vertices of the mesh to be simplified
	* @param [OPTIONAL] quadrics Vertices quadrics for QEM-based representative selection strategy
	* @param [OPTIONAL] vertexWeights Vertex weights for weight-based representative selection strategies
    * 
	* @return unordered map of cell index to representative vertex index for each cell in the grid
    */
    std::unordered_map<uint32_t, uint32_t> computeRepresentativesCellCentre(ClusterGrid& grid, std::vector<Vertex>& vertices);
    std::unordered_map<uint32_t, uint32_t> computeRepresentativesQEM(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<QEM::Quadric>& quadrics);
    std::unordered_map<uint32_t, uint32_t> computeRepresentativesHighestWeight(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<float>& vertexWeights);
    std::unordered_map<uint32_t, uint32_t> computeRepresentativesMeanWeight(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<float>& vertexWeights);

}

 /* End of the VertexClustering.hpp file */