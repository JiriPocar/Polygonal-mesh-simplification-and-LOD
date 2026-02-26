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

namespace Topology {

	struct Neighborhood {
		std::vector<uint32_t> vertices;
		std::vector<uint32_t> triangles;
	};

	Neighborhood getVertexNeighborhood(uint32_t vertexIdx, std::vector<uint32_t>& indices);
	std::vector<Neighborhood> buildAllNeighborhoods(size_t vertexCount, std::vector<uint32_t>& indices);

	std::vector<bool> findBorderVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	bool checkFaceFlipping(glm::vec3 beforePos, glm::vec3 afterPos, uint32_t movingVertexIdx, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);
}

 /* End of the Topology.hpp file */