/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SurfApprox.hpp
 * @brief Utility functions for surface approximation error metrics.
 *
 * This file contains functions to compute distances from points to triangles,
 * one-sided Hausdorff distances, and Mean Squared Error (MSE) between two meshes.
 * 
 * These metrics are furthermore used for evaluating the surface approximation
 * quality of the simplified mesh compared to the original mesh.
 * 
 * @note These metrics CANNOT be reliably used to compare results across different models.
 *       Even with normalization, geometric complexity of models differ — a sphere is trivially
 *       approximable while a model with fine details (e.g. ears, sharp edges) will produce higher
 *		 errors at the same reduction ratio, regardless of algorithm quality.
 * 
 *		 Sampling is only done on vertices of the meshes, which means that the error is not
 *		 measured on the whole surface, but only at set of sampling points (vertices).
 *		 This can lead to underestimation of the error, especially for models with large
 *		 triangles or significant curvature. 
 * 
 *		 Use these metrics accordingly:
 *			- Same model, different algorithm, different reduction ratio
 *			- Same model, different algoritm, same reduction ratio
 *			- Same model, same algorithm, different reduction ratio
 * 
 * =======================================================================================
 * 
 * Inspirations and sources:
 *		- 'distancePointToTriangle' function
 *			- Coming out of "Real-Time Collision Detection" by Christer Ericson, Chapter 5.1.5, "Distance from Point to Triangle", pages 141-142
 *			- @url http://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf
 *		- Hausdorff distance and MSE computations: 
 *			- Only mathematical definitions of Hausdorff distance and MSE
 *			- Coming out of "Quadric-Based Polygonal Surface Simplification" by Michael Garland, Chapter 2.3, page 23, equations (2.7) and (2.8)
 *			- @url https://www.cs.cmu.edu/~garland/thesis/thesis-onscreen.pdf
 * 
 * =======================================================================================
 */

#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <algorithm>
#include "../../resources/Model.hpp"
namespace SurfApprox {

	/**
	* @brief Computes the distance from a point to a triangle in 3D space.
	* 
	* @param p The point from which the distance is measured
	* @param a The first vertex of the triangle
	* @param b The second vertex of the triangle
	* @param c The third vertex of the triangle
	* 
	* @return The shortest distance from the point 'p' to the triangle defined by vertices 'a', 'b', and 'c'.
	* 
	* 
	* This code is taken from the article "Real-Time Collision Detection" by Christer Ericson, published in 2005.
	* The function computes the distance from a point 'p' to a triangle defined by three vertices '(a, b, c)' using Voronoi regions.
	*
	* @author Christer Ericson
	* @in "Real-Time Collision Detection", Chapter 5.1.5, "Distance from Point to Triangle"
	* @pages 141-142
	* @url http://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf
	* @date 2026-03-31
	*/
	float distancePointToTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	/**
	* @brief Computes the one-sided Hausdorff distance from a source mesh to a target mesh.
	* 
	* @param sourceVerts The vertices of the source mesh
	* @param targetVerts The vertices of the target mesh
	* @param targetIndices The triangle indices of the target mesh
	* 
	* @return maximum distance from any vertex in the source mesh to the closest point on the target mesh
	*/
	float computeOneSideDistance(const std::vector<Vertex>& sourceVerts, const std::vector<Vertex>& targetVerts, const std::vector<uint32_t>& targetIndices);

	/**
	* @brief Computes the one-sided Hausdorff distance squared from a source mesh to a target mesh.
	* 
	* @param sourceVerts The vertices of the source mesh
	* @param targetVerts The vertices of the target mesh
	* @param targetIndices The triangle indices of the target mesh
	* 
	* @return sum of squared distances from each vertex in the source mesh to the closest point on the target mesh
	*/
	float computeOneSideSquaredDistance(const std::vector<Vertex>& sourceVerts, const std::vector<Vertex>& targetVerts, const std::vector<uint32_t>& targetIndices);

	/**
	* @brief Computes the Hausdorff distance between two meshes.
	* 
	* @reference "Quadric-Based Polygonal Surface Simplification" by Michael Garland, Chapter 2.3, page 23, equation (2.7)
	* 
	* @param verticesA The vertices of the first mesh
	* @param indicesA The triangle indices of the first mesh
	* @param verticesB The vertices of the second mesh
	* @param indicesB The triangle indices of the second mesh
	* 
	* @return maximum distance from any vertex in either mesh to the closest point on the other mesh
	*/
	float getHausdorffDistance(const std::vector<Vertex>& verticesA, const std::vector<uint32_t>& indicesA, const std::vector<Vertex>& verticesB, const std::vector<uint32_t>& indicesB);

	/**
	* @brief Computes the Mean Squared Error (MSE) between two meshes.
	* 
	* @reference "Quadric-Based Polygonal Surface Simplification" by Michael Garland, Chapter 2.3, page 23, equation (2.8)
	* 
	* @param verticesA The vertices of the first mesh
	* @param indicesA The triangle indices of the first mesh
	* @param verticesB The vertices of the second mesh
	* @param indicesB The triangle indices of the second mesh
	* 
	* @return symmetric mean squared distance from each vertex in the first mesh to the closest point on the second mesh
	*/
	float getMSEError(const std::vector<Vertex>& verticesA, const std::vector<uint32_t>& indicesA, const std::vector<Vertex>& verticesB, const std::vector<uint32_t>& indicesB);
}

/* End of the SurfApprox.hpp file */