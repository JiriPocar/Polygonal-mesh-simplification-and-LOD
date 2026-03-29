/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Geometry.hpp
 * @brief Utility functions for mesh geometry and geometry manipulations.
 *
 * This file contains ...
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
	void computeBounds(std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax);
	float getEdgeLength(const glm::vec3& v1, const glm::vec3& v2);
	bool computeAveragePlane(const std::vector<uint32_t>& orderedLoop, const std::vector<Vertex>& vertices, glm::vec3& outCenter, glm::vec3& outNormal);
	bool isPointInTriangle2D(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

	void remapIndices(std::vector<uint32_t>& indices, std::unordered_map<uint32_t, uint32_t>& vertexRemap);
	void remapIndices(std::vector<uint32_t>& indices, uint32_t oldIdx, uint32_t newIdx);
	void removeDegeneratedTriangles(std::vector<uint32_t>& indices);
	void finalizeVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	void mergeCloseVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, CollapseOptions& options, float threshold = 0.0001f);
	void makeFlatShaded(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	void recalculateSmoothNormals(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
}

 /* End of the Geometry.hpp file */