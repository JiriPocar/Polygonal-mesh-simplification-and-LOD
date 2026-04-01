/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VertexClustering.cpp
 * @brief Vertex clustering algorithm implementation.
 *
 * This file contains the implementation of the vertex clustering algorithm, including
 * the structure of the cluster grid, functions for creating and filling the grid,
 * grading vertices based on their neighborhood, and different strategies for selecting
 * representative vertices for each cluster.
 *
 * =======================================================================================
 *
 * Inspirations and sources:
 *      - The very original Vertex clustering algorithm definition
 *		    - "Multi-resolution 3D approximation for rendering complex scenes" by Jarek Rossignac and Paul Borrel
 *              - @url https://www.researchgate.net/publication/225075920_Multi-resolution_3D_approximation_for_rendering_complex_scenes
 *      - Vertex weight calculation
 *          - "Model Simplification Using Vertex-Clustering" by Kok-Lim Low and Tiow-Seng Tan
 *              - @url https://www.comp.nus.edu.sg/~tants/Paper/simplify.pdf
 *      - Modification using QEM for representative selection
 *          - "Out-of-Core Simplification of Large Polygonal Models" by Peter Lindstrom
 *              - @url http://www-evasion.imag.fr/Membres/Franck.Hetroy/Teaching/Geo3D/Articles/lindstrom2000.pdf
 *
 * =======================================================================================
 */

#include "VertexClustering.hpp"

namespace VertexClustering {

	float computeGridCellSize(std::vector<Vertex>& vertices, size_t cellsPerAxis)
	{
		glm::vec3 minBounds, maxBounds;
		Geometry::computeBounds(vertices, minBounds, maxBounds);

		glm::vec3 size = maxBounds - minBounds;

		float maxAxisLen = std::max({ size.x, size.y, size.z });

		return maxAxisLen / (float)cellsPerAxis;
	}

	ClusterGrid createGrid(std::vector<Vertex>& vertices, float cellSize)
	{
		ClusterGrid grid;
		grid.cellSize = cellSize;
		Geometry::computeBounds(vertices, grid.minBounds, grid.maxBounds);

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

	float calculateVertexWeight(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Topology::Neighborhood& neighborhood)
	{
		// using a grading procedure from Low & Tan (Floating Cell clustering, 1997)
		float maxLength = 0.0f;
		float maxAngle = 0.0f;

		glm::vec3 p0 = vertices[vertexIdx].pos;

		for (size_t i = 0; i < neighborhood.vertices.size(); i++)
		{
			glm::vec3 p1 = vertices[neighborhood.vertices[i]].pos;
			glm::vec3 e1 = p1 - p0;
			float length = glm::length(e1);

			if (length > maxLength) maxLength = length;
			if (length < 1e-6f) continue;

			glm::vec3 direction = e1 / length;

			for (size_t j = i + 1; j < neighborhood.vertices.size(); j++)
			{
				glm::vec3 p2 = vertices[neighborhood.vertices[j]].pos;
				glm::vec3 e2 = p2 - p0;
				float length2 = glm::length(e2);

				if (length2 < 1e-6f) continue;

				glm::vec3 direction2 = e2 / length2;

				float dot = glm::clamp(glm::dot(direction, direction2), -1.0f, 1.0f);
				float angle = std::acos(dot);

				if (angle > maxAngle) maxAngle = angle;
			}
		}

		// weight compute by Low & Tan
		// weight = L_max * cos(thetaMax / 2)
		float weight = maxLength * std::cos(maxAngle / 2.0f);

		return weight;
	}

	std::unordered_map<uint32_t, uint32_t> computeRepresentativesCellCentre(ClusterGrid& grid, std::vector<Vertex>& vertices)
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

	std::unordered_map<uint32_t, uint32_t> computeRepresentativesQEM(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<QEM::Quadric>& quadrics)
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

					// init sum of quadrics with the first vertex in the cell and mass center with its position
					QEM::Quadric sumQ = quadrics[cell[0]];
					glm::vec3 massCenter = vertices[cell[0]].pos;

					// sum quadrics and compute mass center for all vertices in the cell
					for (size_t i = 1; i < cell.size(); i++) {
						sumQ = sumQ + quadrics[cell[i]];
						massCenter += vertices[cell[i]].pos;
					}
					massCenter /= static_cast<float>(cell.size());

					// compute optimal position by minimizing v^T * Q * v
					glm::mat3 MAT(
						sumQ.q11, sumQ.q12, sumQ.q13,
						sumQ.q12, sumQ.q22, sumQ.q23,
						sumQ.q13, sumQ.q23, sumQ.q33
					);

					glm::vec3 optPos;
					float det = glm::determinant(MAT);

					// if regular, compute optimal position by solving the linear system
					if (std::abs(det) > 1e-3f) {
						glm::vec3 b(sumQ.q14, sumQ.q24, sumQ.q34);
						optPos = -glm::inverse(MAT) * b;

						glm::vec3 cellMin = grid.minBounds + glm::vec3(x, y, z) * grid.cellSize;
						glm::vec3 cellMax = cellMin + glm::vec3(grid.cellSize);

						// in rare cases prevent from the optPos being outside of the cell
						if (optPos.x < cellMin.x || optPos.x > cellMax.x ||
							optPos.y < cellMin.y || optPos.y > cellMax.y ||
							optPos.z < cellMin.z || optPos.z > cellMax.z)
						{
							optPos = massCenter;
						}
					}
					else {
						// if singular, fallback to mass center
						optPos = massCenter;
					}

					// remap all other vertices in cell to the vertex with optimal position
					// set its position to optimal position
					uint32_t bestIdx = cell[0];
					vertices[bestIdx].pos = optPos;

					// remap indices
					for (uint32_t idx : cell) {
						if (idx != bestIdx) {
							indexRemap[idx] = bestIdx;
						}
					}
				}
			}
		}

		return indexRemap;
	}

	std::unordered_map<uint32_t, uint32_t> computeRepresentativesHighestWeight(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<float>& vertexWeights)
	{
		std::unordered_map<uint32_t, uint32_t> indexRemap;

		for (int x = 0; x < grid.sizeX; x++) {
			for (int y = 0; y < grid.sizeY; y++) {
				for (int z = 0; z < grid.sizeZ; z++) {
					auto& cell = grid.cells[x][y][z];

					// ignore empty cells
					if (cell.empty()) continue;

					uint32_t bestIdx = cell[0];
					float bestWeight = vertexWeights[bestIdx];

					// iterate through cell to find higher weight vertex
					for (size_t i = 1; i < cell.size(); i++) {
						uint32_t currentIdx = cell[i];

						if (vertexWeights[currentIdx] > bestWeight) {
							bestWeight = vertexWeights[currentIdx];
							bestIdx = currentIdx;
						}
					}

					// remap indices
					for (uint32_t idx : cell) {
						if (idx != bestIdx) {
							indexRemap[idx] = bestIdx;
						}
					}
				}
			}
		}

		return indexRemap;
	}

	std::unordered_map<uint32_t, uint32_t> computeRepresentativesMeanWeight(ClusterGrid& grid, std::vector<Vertex>& vertices, std::vector<float>& vertexWeights)
	{
		std::unordered_map<uint32_t, uint32_t> indexRemap;

		for (int x = 0; x < grid.sizeX; x++)
		{
			for (int y = 0; y < grid.sizeY; y++)
			{
				for (int z = 0; z < grid.sizeZ; z++)
				{
					auto& cell = grid.cells[x][y][z];

					// ignore empty cells
					if (cell.empty()) continue;

					// skip if only one vertex is in the cell
					if (cell.size() == 1) continue;

					glm::vec3 weightedPos(0.0f);
					float totalWeight = 0.0f;

					// sum positions weighted by their weights and sum of weights in the cell
					for (uint32_t idx : cell)
					{
						float w = vertexWeights[idx];
						weightedPos += vertices[idx].pos * w;
						totalWeight += w;
					}

					glm::vec3 finalPos;

					// compute final position as weighted average of positions in the cell
					if (totalWeight > 1e-6f)
					{
						finalPos = weightedPos / totalWeight;
					}
					else
					{
						// fallback if weight is zero for all vertices in the cell - use mean position
						glm::vec3 meanPos(0.0f);
						for (uint32_t idx : cell)
						{
							meanPos += vertices[idx].pos;
						}
						finalPos = meanPos / static_cast<float>(cell.size());
					}

					// choose one vertex in the cell to be the representative and set its position to the computed final position
					uint32_t bestIdx = cell[0];
					vertices[bestIdx].pos = finalPos;

					// remap indices
					for (uint32_t idx : cell) {
						if (idx != bestIdx) {
							indexRemap[idx] = bestIdx;
						}
					}
				}
			}
		}

		return indexRemap;
	}

}

 /* End of the VertexClustering.hpp file */