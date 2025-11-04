#pragma once
#include "../resources/Model.hpp"
#include <vector>
#include <unordered_map>
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

	// Vertex Clustering utilities
	struct ClusterGrid {
		std::vector<std::vector<std::vector<std::vector<uint32_t>>>> cells;
		glm::vec3 minBounds;
		glm::vec3 maxBounds;
		
		float cellSize;
		int sizeX, sizeY, sizeZ;
	};
	void computeBounds(std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax);
	ClusterGrid createGrid(std::vector<Vertex>& vertices, float cellSize);
	void fillGrid(ClusterGrid& grid, std::vector<Vertex>& vertices);
	std::unordered_map<uint32_t, uint32_t> computeClusterCentroids(ClusterGrid& grid, std::vector<Vertex>& vertices);
	float computeGridCellSize(std::vector<Vertex>& vertices, size_t cellsPerAxis);
	// End of Vertex Clustering utilities

	void collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Edge& edgeToCollapse);
	void remapIndices(std::vector<uint32_t>& indices, uint32_t oldIdx, uint32_t newIdx);
	void remapIndices(std::vector<uint32_t>& indices, std::unordered_map<uint32_t, uint32_t>& indexMap);
	void removeDegeneratedTriangles(std::vector<uint32_t>& indices);

} // end of SimplificationUtil namespace