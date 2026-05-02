/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file DualModel.hpp
 * @brief Header file for the DualModel class.
 *
 * This file implements the DualModel class, which is an abstraction
 * for managing both the original and simplified versions of a 3D model
 * for split-screen rendering in the Simplificator application.
 */

#pragma once
#include "Model.hpp"
#include <memory>

class CommandManager;

class DualModel {
public:
	DualModel(Device& device, CommandManager& cmd, const std::string& modelPath);
	~DualModel() = default;

	/**
	* @brief Draws the original model using the provided command buffer.
	* 
	* @param cmd The command buffer to record draw commands into
	*/
	void drawOriginalModel(vk::CommandBuffer cmd) const;

	/**
	* @brief Draws the simplified model using the provided command buffer.
	* 
	* @param cmd The command buffer to record draw commands into
	*/
	void drawSimplifiedModel(vk::CommandBuffer cmd) const;

	/**
	* @brief Simplifies the model by replacing the meshes in the simplified model with new meshes.
	* 
	* @param newMeshesData A vector of MeshData containing the vertex and index data vectors
	*/
	void simplifyModel(const std::vector<MeshData>& newMeshesData);

	// reverts the simplified model back to the original model
	void revertSimplification();

	// getters
	Model& getOriginalModel() const { return *originalModel; }
	Model& getSimplifiedModel() const { return *simplifiedModel; }
	bool wasModelSimplified() const { return wasSimplified; }

private:
	// makes a copy of the original model to be used as the simplified model
	void createSimplifiedCopy();

	Device& dev;
	std::unique_ptr<Model> originalModel;
	std::unique_ptr<Model> simplifiedModel;
	bool wasSimplified = false;
};

/* End of the DualModel.hpp file */