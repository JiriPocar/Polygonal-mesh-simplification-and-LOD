#pragma once
#include "../resources/Model.hpp"
#include <vector>
#include <unordered_set>
#include <glm/glm.hpp>

namespace SimplificationUtil {

	struct Edge {
		uint32_t v1, v2;
		float length;

		bool isIdentical(uint32_t e1, uint32_t e2)
		{
			uint32_t minE = std::min(e1, e2);
			uint32_t maxE = std::max(e1, e2);
			return (v1 == minE && v2 == maxE);
		}
	};

	std::vector<Edge> getEdgesInModel(std::vector<uint32_t>& indices);

	float getEdgeLength(const glm::vec3& v1, const glm::vec3& v2);
	Edge findShortestEdge(std::vector<Vertex>& vertices, std::vector<Edge>& edges);

	void collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Edge& edgeToCollapse);

	void remapIndices(std::vector<uint32_t>& indices, uint32_t oldIdx, uint32_t newIdx);

	void removeDegeneratedTriangles(std::vector<uint32_t>& indices);

} // end of SimplificationUtil namespace