/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Geometry.hpp
 * @brief Utility functions for mesh geometry and geometry manipulations.
 *
 * This file contains various utility functions used for mesh geometry processing and manipulations along
 * with the pure geometry functions. These include functions for computing bounding boxes, edge lengths,
 * average plane or point-in-triangle tests.
 * 
 * Inspirations and adapted codes are specified in the Geometry.cpp file in their respective functions. 
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include "../../resources/Model.hpp"
#include "../Simplificator.hpp"

namespace Geometry {

	/**
	* @brief Computes the min and max corners of the bounding box for the given vertices.
	* 
	* @param vertices The vertices of the mesh to compute the bounding box for
	* @param outMin Output parameter for the minimum corner of the bounding box
	* @param outMax Output parameter for the maximum corner of the bounding box
	*/
	void computeBounds(std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax);
	
	/**
	* @brief Computes the average plane (center and normal) for a given loop of vertices using Newells method.
	* 
	* @param orderedLoop The ordered loop of vertex indices for which to compute the average plane
	* @param vertices The vertices of the mesh to compute the average plane for
	* @param outCenter Output parameter for the center of the average plane
	* @param outNormal Output parameter for the normal of the average plane
	* 
	* @return true if average plane was succesfully computed, false otherwise
	*/
	bool computeAveragePlane(const std::vector<uint32_t>& orderedLoop, const std::vector<Vertex>& vertices, glm::vec3& outCenter, glm::vec3& outNormal);
	
	/**
	* @brief Determines if a point is inside a triangle.
	* 
	* @param p The point to test
	* @param a The first vertex of the triangle
	* @param b The second vertex of the triangle
	* @param c The third vertex of the triangle
	* 
	* @return true if the point is inside the triangle, false otherwise
	*/
	bool isPointInTriangle2D(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

	/**
	* @brief Remaps the indices in the given vector according to the provided vertex remapping.
	* 
	* @param indices The vector of indices to be remapped
	* @param vertexRemap Unordered map, key is the old vertex index and the value is the new vertex index to which it should be remapped
	*/
	void remapIndices(std::vector<uint32_t>& indices, std::unordered_map<uint32_t, uint32_t>& vertexRemap);
	
	/**
	* @brief Remaps the indices in the given vector by replacing all occurrences of oldIdx with newIdx.
	* 
	* @param indices The vector of indices to be remapped
	* @param oldIdx The old vertex index to be replaced
	* @param newIdx The new vertex index to replace with
	*/
	void remapIndices(std::vector<uint32_t>& indices, uint32_t oldIdx, uint32_t newIdx);
	
	/**
	* @brief Removes degenerated triangles in the index vector. Doesnt interact with vertices in any way.
	* 
	* @param indices The vector of indices to remove the degenerated triangles from
	*/
	void removeDegeneratedTriangles(std::vector<uint32_t>& indices);
	
	/**
	* @brief Finalizes the vertex and index buffers after simplification by removing unused vertices
	*		 and remapping indices to the new vertex buffer.
	* 
	* @param vertices The vector of vertices to be finalized
	* @param indices The vector of indices to be finalized
	*/
	void finalizeVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	/**
	* @brief Merges close vertices in 3D space according to options selected.
	* 
	* @note Options can specify whether to merge base on UVs or normals.
	*		UV threshold is 0.001
	*		Normal threshold is 8 degrees (cosine threshold is 0.99)
	* 
	* @param vertices The vector of vertices
	* @param indices The vector of indices
	* @param options The simplification options containing the merge close vertices settings
	* @param threshold The distance threshold for merging vertices, default is 0.0001f
	*/
	void mergeCloseVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, SimplificationOptions& options, float threshold = 0.0001f);
	
	/**
	* @brief Converts the mesh to flat shading by duplicating vertices for each triangle and assigning them the same normal.
	* 
	* @param vertices The vector of vertices
	* @param indices The vector of indices
	*/
	void makeFlatShaded(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	
	/**
	* @brief Recalculates the vertex normals for smooth shading by averaging the face normals of adjacent triangles for each vertex.
	* 
	* @param vertices The vector of vertices
	* @param indices The vector of indices
	*/
	void recalculateSmoothNormals(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
}

 /* End of the Geometry.hpp file */