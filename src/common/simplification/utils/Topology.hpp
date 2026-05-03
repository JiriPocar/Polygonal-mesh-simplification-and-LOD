/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Topology.hpp
 * @brief Utility functions for mesh topology tracking.
 *
 * This file contains various utility functions for topology of meshes. Includes neighborhood
 * creating, face flipping and connectivity checks and twin map building.
 */

#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "common/resources/Model.hpp"
#include "common/simplification/Simplificator.hpp"

namespace Topology {

	struct Neighborhood {
		std::vector<uint32_t> vertices;
		std::vector<uint32_t> triangles;
	};

	/**
	* @brief Builds a neighborhood of a single vertex.
	* 
	* @param vertexIdx The index of the vertex to build the neighborhood for
	* @param indices The index buffer of the whole mesh
	* 
	* @return A Neighborhood struct containing the neighboring vertices and triangles of the given vertex index
	*/
	Neighborhood getVertexNeighborhood(uint32_t vertexIdx, std::vector<uint32_t>& indices);

	/**
	* @brief Builds neighborhoods for all vertices in the mesh.
	* 
	* @param vertexCount The total number of vertices in the mesh
	* @param indices The index buffer of the whole mesh
	* 
	* @return A vector of Neighborhood structs, where each entry at index 'i' corresponds to the neighborhood of vertex with index 'i'
	*/
	std::vector<Neighborhood> buildAllNeighborhoods(size_t vertexCount, std::vector<uint32_t>& indices);

	/**
	* @brief Finds locked vertices of the mesh according to simplification options selected.
	* 
	* @param vertices The vector of vertices in the mesh
	* @param indices The index buffer of the whole mesh
	* @param representatives The vector of vertex representatives of duplicate vertices in the mesh
	* @param options The simplification options containing the settings for locked vertices
	* 
	* @return A vector of booleans, where each entry at index 'i' is true if the vertex with index 'i' is locked and false otherwise
	*/
	std::vector<bool> findLockedVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<uint32_t>& representatives, const SimplificationOptions& options);
	
	/**
	* @brief Checks if moving a vertex from beforePos to afterPos would cause face flipping in any of the adjacent triangles.
	* 
	* @param beforePos The original position of the vertex before the move
	* @param afterPos The new position of the vertex after the move
	* @param movingVertexIdx The index of the vertex being moved
	* @param indices The index buffer of the whole mesh
	* @param vertices The vector of vertices in the mesh
	* @param n The neighborhood of the moving vertex containing its adjacent vertices and triangles
	* 
	* @return true if the move would cause face flipping in any adjacent triangle, false otherwise
	*/
	bool checkFaceFlipping(glm::vec3 beforePos, glm::vec3 afterPos, uint32_t movingVertexIdx, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices, const Neighborhood& n);
	
	/**
	* @brief Checks connectivity of two vertices after collapse.
	* 
	* @param v1 The index of the first vertex
	* @param v2 The index of the second vertex
	* @param indices The index buffer of the whole mesh
	* @param n1 The neighborhood of the first vertex containing its adjacent vertices and triangles
	* @param n2 The neighborhood of the second vertex containing its adjacent vertices and triangles
	* 
	* @return true if the collapse of v1 and v2 would maintain connectivity of the mesh, false otherwise
	*/
	bool checkConnectivity(uint32_t v1, uint32_t v2, std::vector<uint32_t>& indices, const Neighborhood& n1, const Neighborhood& n2);
	
	/**
	* @brief Counts the number of active faces in the mesh after remapping indices to representatives.
	* 
	* @param indices The index buffer of the whole mesh
	* @param reps The vector of vertex representatives of duplicate vertices in the mesh
	* 
	* @return Number of active faces in the mesh
	*/
	size_t countActiveFaces(const std::vector<uint32_t>& indices, const std::vector<uint32_t>& reps);

	/**
	* @brief Builds a vector of vertex representatives for vertices that are in the same place in 3D space.
	* 
	* @param vertices The vector of vertices in the mesh
	* 
	* @return A vector of uint32_t, where each entry at index 'i' is the index of the representative vertex for the vertex with index 'i'.
	*		  Vertices that are in the same place will have the same representative index.
	*/
	std::vector<uint32_t> buildSamePlaceRepresentatives(const std::vector<Vertex>& vertices);

	/**
	* @brief Builds a twin map for vertices that are represented by the same representative vertex.
	* 
	* @param representatives The vector of vertex representatives of duplicate vertices in the mesh
	* 
	* @return A vector of vectors of uint32_t, where each entry at index 'i' is a vector of vertex indices that are twins of the vertex with index 'i'.
	*		  Vertices that are not twins will have an empty vector at their index.
	*/
	std::vector<std::vector<uint32_t>> buildTwinMap(const std::vector<uint32_t>& representatives);
}

 /* End of the Topology.hpp file */