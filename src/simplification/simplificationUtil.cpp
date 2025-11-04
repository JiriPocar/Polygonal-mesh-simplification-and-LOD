#include "simplificationUtil.hpp"
#include <algorithm>
#include <cmath>
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
}