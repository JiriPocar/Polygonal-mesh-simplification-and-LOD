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

	// QEM utilities
	struct Quadric {
		double q11, q12, q13, q14;
		double      q22, q23, q24;
		double           q33, q34;
		double                q44;
		
		Quadric() : q11(0), q12(0), q13(0), q14(0),
							q22(0), q23(0), q24(0),
									q33(0), q34(0),
											q44(1){}

		Quadric operator+(const Quadric& other);
		double evalError(const glm::vec3& v);
	};

	struct Qedge {
		uint32_t v1, v2;
		glm::vec3 optimalPos;
		double error;

		bool operator>(const Qedge& other) const {
			return error > other.error;
		}
	};

	std::vector<Quadric> initQuadrics(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	Quadric createQuadricFromTriangle(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
	glm::vec3 computeOptPos(Quadric& q1, Quadric& q2, glm::vec3& v1, glm::vec3& v2, double& outErr);
	std::vector<Qedge> createQedges(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, std::vector<bool>& isBorderVertex);
	bool getValidMinEdge(std::vector<Qedge>& edges, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Qedge& outEdge);
	void collapseQedge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, Qedge& edge);
	void updateAfterCollapse(std::vector<Qedge>& edges, uint32_t idxToRemove, uint32_t idxToKeep, std::vector<Vertex>& vertices, std::vector<Quadric>& quadrics, std::vector<bool>& isBorderVertex);
	// End of QEM utilities

	std::vector<bool> findBorderVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	bool checkFaceFlipping(glm::vec3 beforePos, glm::vec3 afterPos, uint32_t movingVertexIdx, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);
	void collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Edge& edgeToCollapse);
	void remapIndices(std::vector<uint32_t>& indices, uint32_t oldIdx, uint32_t newIdx);
	void remapIndices(std::vector<uint32_t>& indices, std::unordered_map<uint32_t, uint32_t>& indexMap);
	void removeDegeneratedTriangles(std::vector<uint32_t>& indices);

} // end of SimplificationUtil namespace