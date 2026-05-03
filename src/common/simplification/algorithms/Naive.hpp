/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Naive.hpp
 * @brief Naive algorithm implementation.
 *
 * This file contains helper funtions for naive shortest edge collapse algorithm.
 * Used for comparison with more complex algorithms.
 */

#pragma once
#include <vector>
#include <algorithm>
#include "common/resources/Model.hpp"
#include "common/simplification/utils/Geometry.hpp"
#include "common/simplification/utils/Topology.hpp"

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

    /**
	* @brief Extracts unique edges from the mesh based on the index buffer.
    * 
	* @param indices The index buffer of the mesh
    * 
	* @return A vector of unique edges in the mesh.
    */
    std::vector<Edge> getEdgesInModel(const std::vector<uint32_t>& indices);

    /**
	* @brief Finds the shortest edge in the mesh based on the provided vertices and edges.
    * 
	* @param vertices The vertices of the mesh
	* @param edges The edges of the mesh
    * 
	* @return The shortest edge found in the mesh.
    */
    Edge findShortestEdge(const std::vector<Vertex>& vertices, const std::vector<Edge>& edges);

    /**
	* @brief Collapses the given edge by merging its two vertices and updating the mesh connectivity.
    * 
	* @param vertices The vertices of the mesh
	* @param indices The index buffer of the mesh
	* @param edgeToCollapse The edge to be collapsed
	* @param vertexDeleted A vector of booleans indicating which vertices have been deleted
	* @param allNeighborhoods The neighborhoods of all vertices in the mesh for topology checks
    * 
	* @return The number of triangles removed by the edge collapse operation.
    */
    int collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const Edge& edgeToCollapse, std::vector<bool>& vertexDeleted, std::vector<Topology::Neighborhood>& allNeighborhoods);
    
    /**
	* @brief Checks if collapsing the edge defined by vertices v1 and v2 is valid.
    * 
	* @param v1 The index of the first vertex of the edge
	* @param v2 The index of the second vertex of the edge
	* @param vertices The vertices of the mesh
	* @param indices The index buffer of the mesh
	* @param options The simplification options containing settings for locked vertices and other constraints
	* @param isBorderVertex Vector indicating which vertices are on the border of the mesh
	* @param allNeighborhoods The neighborhoods of all vertices in the mesh for topology checks
    * 
	* @return true if the edge collapse is valid according to the checks, false otherwise.
    */
    bool isCollapseValid(uint32_t v1, uint32_t v2, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, SimplificationOptions& options, const std::vector<bool>& isBorderVertex, std::vector<Topology::Neighborhood>& allNeighborhoods);
}

 /* End of the Naive.hpp file */