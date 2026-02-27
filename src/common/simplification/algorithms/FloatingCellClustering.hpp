/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file FloatingCellClustering.hpp
 * @brief Floating cell clustering algorithm implementation.
 *
 * This file contains ...
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../../resources/Model.hpp"

namespace FloatingCellClustering {
	float computeRadius(std::vector<Vertex>& vertices, size_t cellsPerAxis);

	std::unordered_map<uint32_t, uint32_t> computeRepresentative(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius);
}

 /* End of the FloatingCellCLustering.hpp file */