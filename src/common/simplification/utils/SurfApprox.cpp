/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SurfApprox.cpp
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

#include "SurfApprox.hpp"

namespace SurfApprox {

	float distancePointToTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
	{
		/**
		* This code is taken from the article "Real-Time Collision Detection" by Christer Ericson, published in 2005.
		* The function computes the distance from a point 'p' to a triangle defined by three vertices '(a, b, c)' using Voronoi regions.
		* 
		* @author Christer Ericson
		* @in "Real-Time Collision Detection", Chapter 5.1.5, "Distance from Point to Triangle"
		* @pages 141-142
		* @url http://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf
		* @date 2026-03-31
		*/

		// check if P in vertex region outside A
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 ap = p - a;
		float d1 = glm::dot(ab, ap);
		float d2 = glm::dot(ac, ap);
		if (d1 <= 0.0f && d2 <= 0.0f) return glm::distance(p, a);

		// check if P in vertex region outside B
		glm::vec3 bp = p - b;
		float d3 = glm::dot(ab, bp);
		float d4 = glm::dot(ac, bp);
		if (d3 >= 0.0f && d4 <= d3) return glm::distance(p, b);

		// check if P in edge region of AB, if so return projection of P onto AB
		float vc = d1 * d4 - d3 * d2;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			float v = d1 / (d1 - d3);
			return glm::distance(p, a + v * ab);
		}

		// check if P in vertex region outside C
		glm::vec3 cp = p - c;
		float d5 = glm::dot(ab, cp);
		float d6 = glm::dot(ac, cp);
		if (d6 >= 0.0f && d5 <= d6) return glm::distance(p, c);

		// check if P in edge region of AC, if so return projection of P onto AC
		float vb = d5 * d2 - d1 * d6;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			float w = d2 / (d2 - d6);
			return glm::distance(p, a + w * ac);
		}

		// check if P in edge region of BC, if so return projection of P onto BC
		float va = d3 * d6 - d5 * d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			return glm::distance(p, b + w * (c - b));
		}

		// P inside face region. Compute Q through its barycentric coordinates (u, v, w)
		float denom = 1.0f / (va + vb + vc);
		float v = vb * denom;
		float w = vc * denom;
		return glm::distance(p, a + ab * v + ac * w);
	}

	float computeOneSideDistance(const std::vector<Vertex>& sourceVerts, const std::vector<Vertex>& targetVerts, const std::vector<uint32_t>& targetIndices)
	{
		float maxDist = 0.0f;

		// for each vertex in source mesh
		for (const auto& vertex : sourceVerts)
		{
			float minDist = FLT_MAX;

			// find the closest point on the target mesh by checking distance to each triangle
			for (size_t i = 0; i < targetIndices.size(); i += 3)
			{
				float dist = distancePointToTriangle(vertex.pos, targetVerts[targetIndices[i]].pos, targetVerts[targetIndices[i + 1]].pos, targetVerts[targetIndices[i + 2]].pos);
				minDist = std::min(minDist, dist);
			}

			// update the maximum distance found so far
			maxDist = std::max(maxDist, minDist);
		}

		return maxDist;
	}

	float computeOneSideSquaredDistance(const std::vector<Vertex>& sourceVerts, const std::vector<Vertex>& targetVerts, const std::vector<uint32_t>& targetIndices)
	{
		float squaredDist = 0.0f;

		// for each vertex in source mesh
		for (const auto& vertex : sourceVerts)
		{
			float minDist = FLT_MAX;

			// find the closest point on the target mesh by checking distance to each triangle
			for (size_t i = 0; i < targetIndices.size(); i += 3)
			{
				float dist = distancePointToTriangle(vertex.pos, targetVerts[targetIndices[i]].pos, targetVerts[targetIndices[i + 1]].pos, targetVerts[targetIndices[i + 2]].pos);
				minDist = std::min(minDist, dist);
			}

			// accumulate the squared distance
			squaredDist += minDist * minDist;
		}

		return squaredDist;
	}

	float getHausdorffDistance(const std::vector<Vertex>& verticesA, const std::vector<uint32_t>& indicesA, const std::vector<Vertex>& verticesB, const std::vector<uint32_t>& indicesB)
	{
		// Hausdorff distance is the maximum of the one-sided distances from A to B and from B to A
		float distanceAB = computeOneSideDistance(verticesA, verticesB, indicesB);
		float distanceBA = computeOneSideDistance(verticesB, verticesA, indicesA);

		float maxDist = std::max(distanceAB, distanceBA);
		return maxDist;
	}

	float getMSEError(const std::vector<Vertex>& verticesA, const std::vector<uint32_t>& indicesA, const std::vector<Vertex>& verticesB, const std::vector<uint32_t>& indicesB)
	{
		// MSE is the average of the squared distances from A to B and from B to A
		float sqDistAB = computeOneSideSquaredDistance(verticesA, verticesB, indicesB);
		float sqDistBA = computeOneSideSquaredDistance(verticesB, verticesA, indicesA);

		float mse = (sqDistAB + sqDistBA) / (verticesA.size() + verticesB.size());
		return mse;
	}
}

/* End of the SurfApprox.cpp file */