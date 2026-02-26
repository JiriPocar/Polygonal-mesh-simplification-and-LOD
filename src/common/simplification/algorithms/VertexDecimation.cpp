/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VertexDecimation.cpp
 * @brief Vertex decimation algorithm implementation.
 *
 * This file contains ...
 */

#include "VertexDecimation.hpp"

namespace VertexDecimation {

	double computeVertexError(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info)
	{
		switch (info.classification)
		{
		case VertexClassification::Simple:
		{
			glm::vec3 vPos = vertices[vertexIdx].pos;
			glm::vec3 avgNormal(0.0f);
			glm::vec3 avgCenter(0.0f);

			// get normals and centers of neighboring triangles
			for (uint32_t triangleIdx : info.neighborhood.triangles)
			{
				uint32_t i0 = indices[triangleIdx];
				uint32_t i1 = indices[triangleIdx + 1];
				uint32_t i2 = indices[triangleIdx + 2];

				glm::vec3 p0 = vertices[i0].pos;
				glm::vec3 p1 = vertices[i1].pos;
				glm::vec3 p2 = vertices[i2].pos;

				glm::vec3 cp = glm::cross(p1 - p0, p2 - p0);
				float len = glm::length(cp);

				if (len > 1e-6f)
				{
					avgNormal += cp / len;
				}
				avgCenter += (p0 + p1 + p2) / 3.0f;
			}

			if (glm::length(avgNormal) > 1e-6f)
			{
				avgNormal = glm::normalize(avgNormal);
			}
			else
			{
				return 1e9;
			}
			avgCenter /= static_cast<float>(info.neighborhood.triangles.size());

			// compute distance from vertex to average plane of neighboring triangles
			// should be: d = |N * (V - P)|
			double distance = std::abs(glm::dot(avgNormal, vPos - avgCenter));
			return distance;
		}
		case VertexClassification::Boundary: // TODO: complete this
		{
			// TODO: compute error for boundary vertex
			return 1e9;
		}
		case VertexClassification::Complex:
		case VertexClassification::Undefined:
			return 1e9;

		default:
			return 1e9;
		}
	}

	VertexClassification classifyVertex(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info)
	{
		if (!info.isActive)
		{
			return VertexClassification::Undefined;
		}

		size_t numNeighborTriangles = info.neighborhood.triangles.size();
		size_t numNeighborVertices = info.neighborhood.vertices.size();

		if (numNeighborTriangles == 0)
		{
			return VertexClassification::Complex;
		}

		if (numNeighborTriangles == numNeighborVertices)
		{
			return VertexClassification::Simple;
		}
		else if (numNeighborVertices == numNeighborTriangles + 1)
		{
			return VertexClassification::Boundary;
		}
		else // TODO: resolve other cases (corners, interior edges)
		{
			return VertexClassification::Complex;
		}
	}

	std::vector<VertexInfo> computeVertexInfo(std::vector<uint32_t>& indices, size_t vertexCount)
	{
		std::vector<VertexInfo> vertexInfo(vertexCount);

		// for each triangle
		for (size_t i = 0; i < vertexCount; i++)
		{
			vertexInfo[i].neighborhood = Topology::getVertexNeighborhood(i, indices);
		}

		// remove duplicate neighbor vertices and classify vertices
		for (auto& info : vertexInfo)
		{
			// sort neighbor vertices
			std::sort(info.neighborhood.vertices.begin(), info.neighborhood.vertices.end());

			// remove duplicates
			info.neighborhood.vertices.erase(std::unique(info.neighborhood.vertices.begin(), info.neighborhood.vertices.end()), info.neighborhood.vertices.end());

			info.classification = VertexClassification::Undefined;
			info.isActive = true;
		}

		return vertexInfo;
	}

	std::vector<uint32_t> triangulateHole(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info)
	{
		std::vector<uint32_t> newTriangles;

		// TODO: complete trianglation. most likely via ear clipping.

		return newTriangles;
	}

	void updateLocalTopology(std::vector<VertexInfo>& vertexInfo, std::vector<DecimationCandidate>& candidates,
		std::vector<uint32_t>& newTriangles, std::vector<uint32_t>& indices,
		uint32_t removedVertexIdx, std::vector<Vertex>& vertices)
	{
		// TODO: 
		// a) set vertex as inactive
		// b) update neighbor info for neighboring vertices
		// c) add new triangles to indices
	}
}

 /* End of the VertexDecimation.hpp file */