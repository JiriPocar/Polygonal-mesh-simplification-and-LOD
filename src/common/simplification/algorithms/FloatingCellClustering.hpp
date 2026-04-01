/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file FloatingCellClustering.hpp
 * @brief Floating cell clustering algorithm implementation.
 *
 * This file contains implementation of Vertex clustering modification called
 * Floating cell clustering, which is based on the original vertex clustering
 * algorithm, but uses floating cells instead of fixed 3D grid.
 * 
 * =======================================================================================
 * 
 * Inspirations and sources:
 *		- The very original Vertex clustering algorithm definition
 *		    - "Multi-resolution 3D approximation for rendering complex scenes" by Jarek Rossignac and Paul Borrel
 *              - @url https://www.researchgate.net/publication/225075920_Multi-resolution_3D_approximation_for_rendering_complex_scenes
 *		- Floating cell clustering modification
 *			- "Model Simplification Using Vertex-Clustering" by Kok-Lim Low and Tiow-Seng Tan
 *              - @url https://www.comp.nus.edu.sg/~tants/Paper/simplify.pdf
 *		- Optimization
 *			- Sweep and prune for finding close vertices in 3D space
 *				- @url https://leanrada.com/notes/sweep-and-prune/
 * 
 * =======================================================================================
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../../resources/Model.hpp"

namespace FloatingCellClustering {

	/**
	* @brief Computes the cell radius based on cell size given by cells per longest axis parameter.
	* 
	* @param vertices The vertices of the mesh to be simplified
	* @param cellsPerAxis The number of cells along the longest axis of the model bounding box
	* 
	* @return radius of floating cells, which is half of the cell size
	*/
	float computeRadius(std::vector<Vertex>& vertices, size_t cellsPerAxis);


	/**
	* @brief Computes representative vertices for the floating cells based on the given radius.
	* 
	* @param vertices The vertices of the mesh to be simplified
	* @param indices The indices of the mesh to be simplified
	* @param radius The radius for determining which vertices belong to the same floating cell
	* 
	* @return unordered map of cell index to representative vertex index for each cell in the grid 
	*/
	std::unordered_map<uint32_t, uint32_t> computeRepresentative(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius);
}

 /* End of the FloatingCellCLustering.hpp file */