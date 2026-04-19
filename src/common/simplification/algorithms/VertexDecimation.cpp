/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VertexDecimation.cpp
 * @brief Vertex decimation algorithm implementation.
 *
 * =======================================================================================
 * 
 * Inspirations and sources:
 * 
 * Algorithm is based on "Decimation of Triangle Meshes" by William J. Schroeder et al.
 * at https://www.cs.columbia.edu/~allen/PHOTOPAPERS/Schroeder-etal-sg92.pdf
 *
 * This file implements modified version of the algortihm, partly inspired by the Vertex decimation
 * in the VTK library at https://github.com/Kitware/VTK/blob/master/Filters/Core/vtkDecimatePro.cxx.
 *
 * Kept from the original paper:
 *		- Classification of vertices
 *		- Error metrics based on classification
 *		- Concept of vertex removal, hole triangulation, local topology update
 *		- Feature preservation by not removing 'Complex' vertices implicitly or optionally locking border vertices
 *
 * Following parts of the algorithm are inspired by the VTK library implementation:
 *		- Finalizing classification using feature edge count
 *		- Using a priority queue to prevent multiple passes approach
 *
 * Different approaches to the topics in the original paper
 *		- Hole triangulation: originally "Recursive loop splitting" is used, I chose ear clipping instead
 *			- Coming out of: https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
 *			- For 2D projection, using the Newell's method find an average plane of the polygon and project on it
 *			- Ear is defined by three consecutive vertices in the loop, where any other vertex of the loop is not inside
 *			- Best ear is chosen based on (signed area) / (sum of squared edges) for better quality triangles after trianglutaion
 *		- Maintaining the error order using lazy priority queue instead of multiple passes
 *		- Directed edge graph for classification and getting order loop for triangulation
 * 
 *  =======================================================================================
 */

#include "VertexDecimation.hpp"
#include <unordered_set>

namespace VertexDecimation {

	double computeVertexError(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info, SimplificationOptions& options, std::vector<bool>& isLocked)
	{
		if (isLocked[vertexIdx])
		{
			return DONT_DECIMATE_ERROR;
		}

		if (options.preserveBorders && info.classification == VertexClassification::Boundary)
		{
			return DONT_DECIMATE_ERROR;
		}

		switch (info.classification)
		{
		case VertexClassification::Simple:
		{
			glm::vec3 vPos = vertices[vertexIdx].pos;
			glm::vec3 avgNormal(0.0f);
			glm::vec3 avgCenter(0.0f);

			if (!Geometry::computeAveragePlane(info.orderedLoop, vertices, avgCenter, avgNormal))
			{
				// degenerate case, treat as complex to avoid collapsing
				return DONT_DECIMATE_ERROR;
			}

			// compute distance from vertex to average plane of neighboring triangles
			// should be: d = |N * (V - P)|
			double distance = std::abs(glm::dot(avgNormal, vPos - avgCenter));
			return distance;
		}
		case VertexClassification::Boundary:
		{
			if (info.orderedLoop.size() < 2)
			{
				return DONT_DECIMATE_ERROR;
			}

			glm::vec3 vPos = vertices[vertexIdx].pos;
			glm::vec3 edgeStart = vertices[info.orderedLoop.front()].pos;
			glm::vec3 edgeEnd = vertices[info.orderedLoop.back()].pos;

			// compute distance from vertex to the boundary edge (line segment)
			glm::vec3 edge = edgeEnd - edgeStart;
			float edgeLength = glm::length(edge);

			if (edgeLength < 1e-6f)
			{
				// degenerate edge, treat as complex to avoid collapsing
				return DONT_DECIMATE_ERROR;
			}

			// distance from point to line can be computed using the cross product
			// d = |(V - P1) x (P2 - P1)| / |P2 - P1|
			glm::vec3 w = vPos - edgeStart;
			glm::vec3 cp = glm::cross(w, edge);

			double distance = glm::length(cp) / edgeLength;
			return distance;
		}
		case VertexClassification::InteriorEdge:
		{
			// TODO: think about whether to do this
			// for now, treat as complex to avoid collapsing
			return DONT_DECIMATE_ERROR;
		}
		case VertexClassification::Complex:
		case VertexClassification::Corner:
		case VertexClassification::Undefined:
			return DONT_DECIMATE_ERROR;

		default:
			return DONT_DECIMATE_ERROR;
		}
	}

	VertexClassification classifyVertex(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info, SimplificationOptions& options)
	{
		if (!info.isActive)
		{
			return VertexClassification::Undefined;
		}

		if (info.neighborhood.triangles.empty())
		{
			return VertexClassification::Complex;
		}

		// for each CCW triangle (V, a, b): in the fan around V, vertex 'b' follows 'a'
		// next[a] = b
		std::unordered_map<uint32_t, uint32_t> next;

		// vertices that have an incoming edge in the fan
		std::unordered_set<uint32_t> hasIncoming;

		for (uint32_t triIdx : info.neighborhood.triangles)
		{
			uint32_t i0 = indices[triIdx];
			uint32_t i1 = indices[triIdx + 1];
			uint32_t i2 = indices[triIdx + 2];

			uint32_t a;
			uint32_t b;

			if (i0 == vertexIdx)
			{
				a = i1;
				b = i2;
			}
			else if (i1 == vertexIdx)
			{
				a = i2;
				b = i0;
			}
			else
			{
				a = i0;
				b = i1;
			}

			// degenerated triangle
			if (a == b)
			{
				return VertexClassification::Complex;
			}

			// two outcoming edges from 'a' -> non-manifold
			if (next.count(a))
			{
				return VertexClassification::Complex;
			}

			// two incoming edges to 'b' -> non-manifold
			if (!hasIncoming.insert(b).second)
			{
				return VertexClassification::Complex;
			}

			// set that 'a' is pointing at 'b' in the fan
			next[a] = b;
		}

		// find the start of the fan
		uint32_t startNode = next.begin()->first;
		int startCount = 0;
		for (auto& [a, b] : next)
		{
			if (!hasIncoming.count(a))
			{
				startNode = a;
				startCount++;
			}
		}

		// prevent from multiple start nodes
		if (startCount > 1)
		{
			return VertexClassification::Complex;
		}

		// go through the fan and build the ordered list of neighboring vertices in the fan
		std::vector<uint32_t> orderedLoop;
		uint32_t current = startNode;

		while (true)
		{
			orderedLoop.push_back(current);

			auto nextStop = next.find(current);
			if (nextStop == next.end()) // no outgoing edge, reached the end of the fan
			{
				break;
			}

			current = nextStop->second;
			if (current == startNode) // came back to the start
			{
				break;
			}

			// prevent infinite loop in case of non-manifold fan with a cycle
			if (orderedLoop.size() > next.size())
			{
				return VertexClassification::Complex;
			}
		}

		if (orderedLoop.size() != next.size() + startCount)
		{
			return VertexClassification::Complex;
		}

		info.orderedLoop = orderedLoop;

		VertexClassification baseClassification;
		if (startCount == 0)
		{
			// closed fan, vertex is not on boundary
			baseClassification = VertexClassification::Simple;
		}
		else
		{
			// open fan, vertex is on boundary
			baseClassification = VertexClassification::Boundary;
		}

		int featureEdgeCount = 0;
		float featureAngleCos = std::cos(glm::radians(options.featureAngleThreshold));

		for (uint32_t neighbor : info.orderedLoop)
		{
			std::vector<uint32_t> sharedTris;
			for (uint32_t triIdx : info.neighborhood.triangles)
			{
				uint32_t i0 = indices[triIdx];
				uint32_t i1 = indices[triIdx + 1];
				uint32_t i2 = indices[triIdx + 2];
				if (i0 == neighbor || i1 == neighbor || i2 == neighbor)
				{
					sharedTris.push_back(triIdx);
				}
			}

			if (sharedTris.size() == 2)
			{
				// get normal of the first shared triangle
				uint32_t tri0 = sharedTris[0];
				glm::vec3 p0 = vertices[indices[tri0]].pos;
				glm::vec3 p1 = vertices[indices[tri0 + 1]].pos;
				glm::vec3 p2 = vertices[indices[tri0 + 2]].pos;
				glm::vec3 cross0 = glm::cross(p1 - p0, p2 - p0);
				float len0 = glm::length(cross0);
				glm::vec3 n0;
				if (len0 > 1e-6f)
				{
					n0 = cross0 / len0;
				}
				else
				{
					n0 = glm::vec3(0.0f);
				}

				// get normal of the second shared triangle
				uint32_t tri1 = sharedTris[1];
				p0 = vertices[indices[tri1]].pos;
				p1 = vertices[indices[tri1 + 1]].pos;
				p2 = vertices[indices[tri1 + 2]].pos;
				glm::vec3 cross1 = glm::cross(p1 - p0, p2 - p0);
				float len1 = glm::length(cross1);
				glm::vec3 n1;
				if (len1 > 1e-6f)
				{
					n1 = cross1 / len1;
				}
				else
				{
					n1 = glm::vec3(0.0f);
				}

				// increment if angle between normals is greater than the threshold
				if (glm::length(n0) == 0.0f || glm::length(n1) == 0.0f || glm::dot(n0, n1) < featureAngleCos)
				{
					featureEdgeCount++;
				}
			}
		}

		// ============================================================================
		// The following topological promotion logic is an adaptation from
		// the vtkDecimatePro class in the Visualization Toolkit (VTK).
		// 
		// Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
		// SPDX-License-Identifier: BSD-3-Clause
		// Source: https://github.com/Kitware/VTK/blob/master/Filters/Core/vtkDecimatePro.cxx
		// Lines: 820-853
		// Date: 29-03-2026
		// ============================================================================
		if (baseClassification == VertexClassification::Simple)
		{
			if (featureEdgeCount == 0)
			{
				return VertexClassification::Simple;
			}

			if (featureEdgeCount == 2)
			{
				return VertexClassification::InteriorEdge;
			}

			return VertexClassification::Corner;
		}
		else if (baseClassification == VertexClassification::Boundary)
		{
			if (featureEdgeCount == 0)
			{
				return VertexClassification::Boundary;
			}
			else if (featureEdgeCount == 1)
			{
				return VertexClassification::Corner;
			}
			else
			{
				return VertexClassification::Complex;
			}
		}
		// ============================================================================

		return baseClassification;
	}

	std::vector<VertexInfo> computeVertexInfo(std::vector<uint32_t>& indices, size_t vertexCount)
	{
		std::vector<VertexInfo> vertexInfo(vertexCount);

		// for each vertex build its neighborhood
		for (size_t i = 0; i < vertexCount; i++)
		{
			vertexInfo[i].neighborhood = Topology::getVertexNeighborhood(i, indices);
		}

		// remove duplicate neighbor vertices and classify vertices
		for (auto& info : vertexInfo)
		{
			// sort neighbor vertices and remove duplicates
			std::sort(info.neighborhood.vertices.begin(), info.neighborhood.vertices.end());
			info.neighborhood.vertices.erase(std::unique(info.neighborhood.vertices.begin(), info.neighborhood.vertices.end()), info.neighborhood.vertices.end());

			info.classification = VertexClassification::Undefined;
			info.isActive = true;
		}

		return vertexInfo;
	}

	std::vector<uint32_t> triangulateHole(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info)
	{
		std::vector<uint32_t> newTriangles;

		glm::vec3 center;
		glm::vec3 normal;
		if (!Geometry::computeAveragePlane(info.orderedLoop, vertices, center, normal))
		{
			// cant triangulate degenerate hole, return empty
			return std::vector<uint32_t>();
		}

		// create orthonormal basis for projecting to 2D
		glm::vec3 refDir = vertices[info.orderedLoop[0]].pos - center;
		glm::vec3 axisY = glm::normalize(glm::cross(normal, refDir));
		if (glm::length(axisY) < 1e-6f) // fallback if normal is parallel to refDir
		{
			axisY = glm::normalize(glm::cross(normal, glm::vec3(0, 1, 0)));
		}
		glm::vec3 axisX = glm::cross(axisY, normal);

		// project the 3D polygon to 2D
		std::vector<glm::vec2> poly2D;
		for (uint32_t idx : info.orderedLoop)
		{
			// translate vertex to 0,0 and then project to 2D using the orthonormal basis
			glm::vec3 p = vertices[idx].pos - center;
			// dot product with axisX and axisY gives us the coordinates in the 2D plane defined by the average normal
			poly2D.push_back(glm::vec2(glm::dot(p, axisX), glm::dot(p, axisY)));
		}

		// ear clipping
		std::vector<uint32_t> activeIndices = info.orderedLoop;
		std::vector<glm::vec2> activePoly2D = poly2D;

		// need at least 3 vertices to form a triangle
		while (activeIndices.size() > 2)
		{
			int bestEarIdx = -1;
			float bestEarScore = -1.0f;
			int n = activeIndices.size();

			// evaluate all potential ears and find the best one based on geometric quality
			for (int i = 0; i < n; i++)
			{
				int prev = (i - 1 + n) % n;
				int curr = i;
				int next = (i + 1) % n;

				// vertices of the potential ear triangle in 2D
				glm::vec2 pPrev = activePoly2D[prev];
				glm::vec2 pCurr = activePoly2D[curr];
				glm::vec2 pNext = activePoly2D[next];

				// compute signed area to check if triangle is convex (positive means convex in ccw)
				float doubleSignedArea = (pCurr.x - pPrev.x) * (pNext.y - pCurr.y)
									   - (pCurr.y - pPrev.y) * (pNext.x - pCurr.x);

				// prevent non-convex
				if (doubleSignedArea <= 1e-5f)
				{
					continue;
				}

				// check if any other vertex is inside the triangle formed by [prev, curr, next]
				bool isEar = true;
				for (int j = 0; j < n; j++)
				{
					// skip the vertices of the triangle itself
					if (j == prev || j == curr || j == next)
					{
						continue;
					}

					if (Geometry::isPointInTriangle2D(activePoly2D[j], pPrev, pCurr, pNext))
					{
						isEar = false;
						break;
					}
				}

				// compute the ear quality, prioritize ears with highest (area)/(sum of edges^2)
				if (isEar)
				{
					glm::vec2 e1 = pCurr - pPrev;
					glm::vec2 e2 = pNext - pCurr;
					glm::vec2 e3 = pPrev - pNext;

					float eSquared = glm::dot(e1, e1) + glm::dot(e2, e2) + glm::dot(e3, e3);
					float score = doubleSignedArea / eSquared;

					if (score > bestEarScore)
					{
						bestEarScore = score;
						bestEarIdx = curr;
					}
				}
			}

			// cut out the best ear
			if (bestEarIdx != -1)
			{
				int prev = (bestEarIdx - 1 + n) % n;
				int next = (bestEarIdx + 1) % n;

				// add the triangle formed by [prev, curr, next]
				newTriangles.push_back(activeIndices[prev]);
				newTriangles.push_back(activeIndices[bestEarIdx]);
				newTriangles.push_back(activeIndices[next]);

				// remove the ear vertex from the active list
				activeIndices.erase(activeIndices.begin() + bestEarIdx);
				activePoly2D.erase(activePoly2D.begin() + bestEarIdx);
			}
			else // something went wrong here
			{
				break;
			}
		}

		// if the triangulation wasnt complete, return empty to mark this vertex as complex
		if (activeIndices.size() > 2)
		{
			return std::vector<uint32_t>();
		}

		return newTriangles;
	}

	void updateLocalTopology(std::vector<VertexInfo>& vertexInfo, LazyPriorityQueue<DecimationCandidate, DecimationCompare>& candidatesQueue, std::vector<uint32_t>& newTriangles, std::vector<uint32_t>& indices, uint32_t removedVertexIdx, std::vector<Vertex>& vertices, SimplificationOptions& options, std::vector<bool>& isLocked)
	{
		// mark the vertex as inactive
		vertexInfo[removedVertexIdx].isActive = false;

		auto& oldTriOffsets = vertexInfo[removedVertexIdx].neighborhood.triangles;
		std::vector<uint32_t> usedTriOffsets;

		// replace the old triangles with the new ones
		size_t newTriCount = newTriangles.size() / 3;
		for (size_t i = 0; i < newTriCount; i++)
		{
			uint32_t triOffset = oldTriOffsets[i];

			indices[triOffset] = newTriangles[i * 3];
			indices[triOffset + 1] = newTriangles[i * 3 + 1];
			indices[triOffset + 2] = newTriangles[i * 3 + 2];

			usedTriOffsets.push_back(triOffset);
		}

		// degenerate the remaining old triangles
		for (size_t i = newTriCount; i < oldTriOffsets.size(); i++)
		{
			uint32_t triOffset = oldTriOffsets[i];

			indices[triOffset] = 0;
			indices[triOffset + 1] = 0;
			indices[triOffset + 2] = 0;
		}

		// only vertices in orderedLoop on the boundary of the hole are affected
		for (uint32_t boundaryVertex : vertexInfo[removedVertexIdx].orderedLoop)
		{
			// get the vertex info of this neighbor
			auto& vInfo = vertexInfo[boundaryVertex];

			// skip if inactive
			if (!vInfo.isActive) continue;

			// remove triangles that got removed by vertex removal
			vInfo.neighborhood.triangles.erase(
				std::remove_if(vInfo.neighborhood.triangles.begin(), vInfo.neighborhood.triangles.end(),
					[&](uint32_t t) {
						return std::find(oldTriOffsets.begin(), oldTriOffsets.end(), t) != oldTriOffsets.end();
					}),
				vInfo.neighborhood.triangles.end());

			// add the new traingles that got added by hole triangulation
			// we only add those that are actually connected to this boundary vertex
			for (uint32_t triOffset : usedTriOffsets)
			{
				if (indices[triOffset] == boundaryVertex || indices[triOffset + 1] == boundaryVertex || indices[triOffset + 2] == boundaryVertex)
				{
					vInfo.neighborhood.triangles.push_back(triOffset);
				}
			}

			// rebuild the vertex neighborhood vertices from the updated triangle list
			vInfo.neighborhood.vertices.clear();
			for (uint32_t tri : vInfo.neighborhood.triangles)
			{
				uint32_t i0 = indices[tri];
				uint32_t i1 = indices[tri + 1];
				uint32_t i2 = indices[tri + 2];

				// ignore degenerated triangles
				if (i0 != i1 && i1 != i2 && i0 != i2)
				{
					if (i0 != boundaryVertex) vInfo.neighborhood.vertices.push_back(i0);
					if (i1 != boundaryVertex) vInfo.neighborhood.vertices.push_back(i1);
					if (i2 != boundaryVertex) vInfo.neighborhood.vertices.push_back(i2);
				}
			}
			// remove duplicates
			std::sort(vInfo.neighborhood.vertices.begin(), vInfo.neighborhood.vertices.end());
			vInfo.neighborhood.vertices.erase(std::unique(vInfo.neighborhood.vertices.begin(), vInfo.neighborhood.vertices.end()), vInfo.neighborhood.vertices.end());

			// redo classification and error computation
			vInfo.classification = classifyVertex(boundaryVertex, vertices, indices, vInfo, options);
			if (vInfo.classification != VertexClassification::Complex && vInfo.classification != VertexClassification::Undefined)
			{
				double newError = computeVertexError(boundaryVertex, vertices, indices, vInfo, options, isLocked);

				// re-add to the priority queue with the new error
				candidatesQueue.push({ boundaryVertex, newError });
			}
		}
	}
}

 /* End of the VertexDecimation.hpp file */