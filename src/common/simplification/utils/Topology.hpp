/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Topology.hpp
 * @brief Utility functions for mesh topology tracking.
 *
 * This file contains ...
 */

#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "../../resources/Model.hpp"
#include "../Simplificator.hpp"

namespace Topology {

	struct Neighborhood {
		std::vector<uint32_t> vertices;
		std::vector<uint32_t> triangles;
	};

	Neighborhood getVertexNeighborhood(uint32_t vertexIdx, std::vector<uint32_t>& indices);
	std::vector<Neighborhood> buildAllNeighborhoods(size_t vertexCount, std::vector<uint32_t>& indices);

	std::vector<bool> findBorderVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<uint32_t>& representatives);
	bool checkFaceFlipping(glm::vec3 beforePos, glm::vec3 afterPos, uint32_t movingVertexIdx, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);
	bool checkConnectivity(uint32_t v1, uint32_t v2, std::vector<uint32_t>& indices);

	std::vector<uint32_t> buildSamePlaceRepresentatives(const std::vector<Vertex>& vertices);
	std::vector<std::vector<uint32_t>> buildTwinMap(const std::vector<uint32_t>& representatives);

	bool isCollapseValid(uint32_t v1, uint32_t v2,
		std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
		CollapseOptions& options, const std::vector<bool>& isBorderVertex, const std::vector<std::vector<uint32_t>>& twinMap);

	bool isCollapseValidQEM(uint32_t v1, uint32_t v2, glm::vec3 optimalPos,
		std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
		CollapseOptions& options, const std::vector<bool>& isBorderVertex);
}

 /* End of the Topology.hpp file */