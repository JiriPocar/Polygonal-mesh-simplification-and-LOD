#include "simplificationUtil.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <iostream>

namespace SimplificationUtil {

	float getEdgeLength(const glm::vec3& v1, const glm::vec3& v2)
	{
		return glm::length(v2 - v1);
	}

	void remapIndices(std::vector<uint32_t>& indices, uint32_t oldIdx, uint32_t newIdx)
	{
		for (auto& index : indices) {
			if (index == oldIdx) {
				index = newIdx;
			}
		}
	}

	void remapIndices(std::vector<uint32_t>& indices, std::unordered_map<uint32_t, uint32_t>& indexMap)
	{
		for (auto& idx : indices)
		{
			auto it = indexMap.find(idx);
			if (it != indexMap.end())
			{
				idx = it->second;
			}
		}
	}

	std::vector<Edge> getEdgesInModel(std::vector<uint32_t>& indices)
	{
		std::vector<Edge> resultEdges;

		// for each triplet of indices (each triangle)
		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t triplet[3] = { indices[i], indices[i + 1], indices[i + 2] };

			// for each edge in the triangle
			for (int j = 0; j < 3; j++)
			{
				Edge e;
				e.v1 = std::min(triplet[j], triplet[(j + 1) % 3]);
				e.v2 = std::max(triplet[j], triplet[(j + 1) % 3]);

				// check if edge was already added
				bool exists = false;
				for (auto& existingEdge : resultEdges)
				{
					if (existingEdge.v1 == e.v1 && existingEdge.v2 == e.v2)
					{
						exists = true;
						break;
					}
				}

				// if not, add it
				if (!exists)
				{
					resultEdges.push_back(e);
				}
			}
		}

		return resultEdges;
	}

	Edge findShortestEdge(std::vector<Vertex>& vertices, std::vector<Edge>& edges)
	{
		Edge shortestEdge;
		float minLen = FLT_MAX;

		for (auto& edge : edges)
		{
			float len = getEdgeLength(vertices[edge.v1].pos, vertices[edge.v2].pos);
			if (len < minLen)
			{
				minLen = len;
				shortestEdge = edge;
			}
		}

		// TODO: resolve empty model
		return shortestEdge;
	}

	void removeDegeneratedTriangles(std::vector<uint32_t>& indices)
	{
		std::vector<uint32_t> cleanedIndices;

		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t i1 = indices[i];
			uint32_t i2 = indices[i + 1];
			uint32_t i3 = indices[i + 2];

			if (i1 != i2 && i2 != i3 && i1 != i3)
			{
				cleanedIndices.push_back(i1);
				cleanedIndices.push_back(i2);
				cleanedIndices.push_back(i3);
			}
		}
		indices = cleanedIndices;
	}

	void collapseEdge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Edge& edgeToCollapse)
	{
		uint32_t keepIdx = edgeToCollapse.v1;
		uint32_t removeIdx = edgeToCollapse.v2;

		remapIndices(indices, removeIdx, keepIdx);
		removeDegeneratedTriangles(indices);

	}

	void computeBounds(std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax)
	{
		outMin = glm::vec3(FLT_MAX);
		outMax = glm::vec3(-FLT_MAX);

		for (auto& v : vertices)
		{
			outMin = glm::min(outMin, v.pos);
			outMax = glm::max(outMax, v.pos);
		}
	}

	ClusterGrid createGrid(std::vector<Vertex>& vertices, float cellSize)
	{
		ClusterGrid grid;
		grid.cellSize = cellSize;
		computeBounds(vertices, grid.minBounds, grid.maxBounds);

		glm::vec3 size = grid.maxBounds - grid.minBounds;
		grid.sizeX = std::max(1, (int)std::ceil(size.x / cellSize));
		grid.sizeY = std::max(1, (int)std::ceil(size.y / cellSize));
		grid.sizeZ = std::max(1, (int)std::ceil(size.z / cellSize));

		// alloc grid
		grid.cells.resize(grid.sizeX);
		for (int x = 0; x < grid.sizeX; x++) {
			grid.cells[x].resize(grid.sizeY);
			for (int y = 0; y < grid.sizeY; y++) {
				grid.cells[x][y].resize(grid.sizeZ);
			}
		}

		return grid;
	}

	void fillGrid(ClusterGrid& grid, std::vector<Vertex>& vertices)
	{
		for (uint32_t i = 0; i < vertices.size(); i++)
		{
			glm::vec3 relPos = vertices[i].pos - grid.minBounds;

			int x = (int)(relPos.x / grid.cellSize);
			int y = (int)(relPos.y / grid.cellSize);
			int z = (int)(relPos.z / grid.cellSize);

			x = std::clamp(x, 0, grid.sizeX - 1);
			y = std::clamp(y, 0, grid.sizeY - 1);
			z = std::clamp(z, 0, grid.sizeZ - 1);

			grid.cells[x][y][z].push_back(i);
		}
	}

	std::unordered_map<uint32_t, uint32_t> computeClusterCentroids(ClusterGrid& grid, std::vector<Vertex>& vertices)
	{
		std::unordered_map<uint32_t, uint32_t> indexRemap;

		for (int x = 0; x < grid.sizeX; x++)
		{
			for (int y = 0; y < grid.sizeY; y++)
			{
				for (int z = 0; z < grid.sizeZ; z++)
				{
					auto& cell = grid.cells[x][y][z];
					if (cell.size() <= 1) continue;

					glm::vec3 cellCenter = grid.minBounds + glm::vec3(
						(x + 0.5f) * grid.cellSize,
						(y + 0.5f) * grid.cellSize,
						(z + 0.5f) * grid.cellSize
					);
					
					// find closest vertex to cell center
					uint32_t bestIdx = cell[0];
					float bestDist = glm::length(vertices[bestIdx].pos - cellCenter);

					for (uint32_t idx : cell)
					{
						float dist = glm::distance(vertices[idx].pos, cellCenter);
						if (dist < bestDist)
						{
							bestDist = dist;
							bestIdx = idx;
						}
					}

					// remap all other vertices in cell to bestIdx
					for (uint32_t idx : cell)
					{
						if (idx != bestIdx)
						{
							indexRemap[idx] = bestIdx;
						}
					}
				}
			}
		}

		return indexRemap;
	}

	float computeGridCellSize(std::vector<Vertex>& vertices, size_t cellsPerAxis)
	{
		glm::vec3 minBounds, maxBounds;
		computeBounds(vertices, minBounds, maxBounds);

		glm::vec3 size = maxBounds - minBounds;

		float maxAxisLen = std::max({size.x, size.y, size.z});

		return maxAxisLen / (float)cellsPerAxis;
	}

	Quadric Quadric::operator+(const Quadric& other)
	{
		Quadric result;
		result.q11 = q11 + other.q11;
		result.q12 = q12 + other.q12;
		result.q13 = q13 + other.q13;
		result.q14 = q14 + other.q14;
		result.q22 = q22 + other.q22;
		result.q23 = q23 + other.q23;
		result.q24 = q24 + other.q24;
		result.q33 = q33 + other.q33;
		result.q34 = q34 + other.q34;
		result.q44 = q44 + other.q44;
		return result;
	}

	double Quadric::evalError(const glm::vec3& v)
	{
		double x = v.x;
		double y = v.y;
		double z = v.z;

		// evaluate v^T * Q * v
		// via https://www.cs.cmu.edu/~garland/Papers/quadrics.pdf [page 3 footnote]
		double error = q11 * x * x + 2 * q12 * x * y + 2 * q13 * x * z + 2 * q14 * x +
										 q22 * y * y + 2 * q23 * y * z + 2 * q24 * y +
														   q33 * z * z + 2 * q34 * z +
																			 q44;

		return error;
	}

	Quadric createQuadricFromTriangle(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
	{
		// get plane normal
		glm::vec3 n = glm::normalize(glm::cross(v2 - v1, v3 - v1));
		double a = n.x, b = n.y, c = n.z;
		double d = -glm::dot(n, v1);

		Quadric q;
		q.q11 = a * a;	q.q12 = a * b;	q.q13 = a * c;	q.q14 = a * d;
						q.q22 = b * b;	q.q23 = b * c;	q.q24 = b * d;
										q.q33 = c * c;	q.q34 = c * d;
														q.q44 = d * d;

		return q;
	}

	std::vector<Quadric> initQuadrics(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<Quadric> quadrics(vertices.size());

		// for each triangle
		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t i1 = indices[i];
			uint32_t i2 = indices[i + 1];
			uint32_t i3 = indices[i + 2];

			glm::vec3& v1 = vertices[i1].pos;
			glm::vec3& v2 = vertices[i2].pos;
			glm::vec3& v3 = vertices[i3].pos;

			Quadric q = createQuadricFromTriangle(v1, v2, v3);

			quadrics[i1] = quadrics[i1] + q;
			quadrics[i2] = quadrics[i2] + q;
			quadrics[i3] = quadrics[i3] + q;
		}

		return quadrics;
	}

	glm::vec3 computeOptPos(Quadric& q1, Quadric& q2, glm::vec3& v1, glm::vec3& v2, double& outErr)
	{
		Quadric q = q1 + q2;

		// for now lets only choose between v1, v2 and midpoint
		// TODO: definitely add more candidates OR compute exact optimal pos
		glm::vec3 middle = 0.5f * (v1 + v2);

		double v1Err = q.evalError(v1);
		double v2Err = q.evalError(v2);
		double midErr = q.evalError(middle);

		if (v1Err <= v2Err && v1Err <= midErr)
		{
			outErr = v1Err;
			return v1;
		}
		else if (v2Err <= v1Err && v2Err <= midErr)
		{
			outErr = v2Err;
			return v2;
		}
		else
		{
			outErr = midErr;
			return middle;
		}
	}

	std::vector<Qedge> createQedges(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics)
	{
		std::set<std::pair<uint32_t, uint32_t>> uniqueEdges;

		// find unique edges
		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t idxTriplet[3] = { indices[i], indices[i + 1], indices[i + 2] };

			for (int j = 0; j < 3; j++)
			{
				uint32_t v1 = idxTriplet[j];
				uint32_t v2 = idxTriplet[(j + 1) % 3];

				auto edge = std::minmax(v1, v2);
				uniqueEdges.insert(edge);
			}
		}

		// create Qedges
		std::vector<Qedge> qedges;
		qedges.reserve(uniqueEdges.size());

		for (auto& [v1, v2] : uniqueEdges)
		{
			Qedge qe;
			qe.v1 = v1;
			qe.v2 = v2;

			qe.optimalPos = computeOptPos(quadrics[v1], quadrics[v2], vertices[v1].pos, vertices[v2].pos, qe.error);
			qedges.push_back(qe);
		}

		return qedges;
	}

	Qedge findMinErr(std::vector<Qedge>& edges)
	{
		if (edges.empty())
		{
			throw std::runtime_error("No edges to find minimum from.");
		}

		auto minIt = std::min_element(edges.begin(), edges.end(),
			[](const Qedge& a, const Qedge& b) {
				return a.error < b.error;
			});

		return *minIt;
	}

	void collapseQedge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, Qedge& edge)
	{
		uint32_t keepIdx = edge.v1;
		uint32_t removeIdx = edge.v2;

		// update vertex position with optimal position
		vertices[keepIdx].pos = edge.optimalPos;

		// update quadric
		quadrics[keepIdx] = quadrics[keepIdx] + quadrics[removeIdx];

		// remap indices
		for (auto& idx : indices)
		{
			if (idx == removeIdx) {
				idx = keepIdx;
			}
		}

		// at last, remove degenerated triangles
		removeDegeneratedTriangles(indices);
	}

	void updateAfterCollapse(std::vector<Qedge>& edges, uint32_t idxToRemove, uint32_t idxToKeep, std::vector<Vertex>& vertices, std::vector<Quadric>& quadrics)
	{
		// remap edges
		for (auto& edge : edges)
		{
			if (edge.v1 == idxToRemove) edge.v1 = idxToKeep;
			if (edge.v2 == idxToRemove) edge.v2 = idxToKeep;
		}

		// delete edges that involved idxToRemove or are now degenerated
		edges.erase(
			std::remove_if(edges.begin(), edges.end(),
				[idxToKeep](const Qedge& e) {
					// delete edge if it is degenerated
					return e.v1 == idxToKeep && e.v2 == idxToKeep;
				}),
			edges.end()
		);

		// recompute edges involving idxToKeep
		for (auto& edge : edges)
		{
			if (edge.v1 == idxToKeep || edge.v2 == idxToKeep)
			{
				uint32_t otherIdx = (edge.v1 == idxToKeep) ? edge.v2 : edge.v1;
				edge.optimalPos = computeOptPos(quadrics[idxToKeep], quadrics[otherIdx], vertices[idxToKeep].pos, vertices[otherIdx].pos, edge.error);
			}
		}
	}
}