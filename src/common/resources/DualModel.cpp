/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file DualModel.cpp
 * @brief Header file for the DualModel class.
 *
 * This file implements an abstraction for managing
 * both the original and simplified versions of a 3D model.
 */

#include "DualModel.hpp"
#include <iostream>

DualModel::DualModel(Device& device, CommandManager& cmd, const std::string& modelPath)
	: dev(const_cast<Device&>(device))
{
	originalModel = std::make_unique<Model>(device, cmd, modelPath);
	createSimplifiedCopy();
}

void DualModel::createSimplifiedCopy()
{
	if (!originalModel)
	{
		throw std::runtime_error("Original model is not loaded.");
	}

	simplifiedModel = std::make_unique<Model>(*originalModel);
	wasSimplified = false;
}

void DualModel::drawOriginalModel(vk::CommandBuffer cmd) const
{
	if (originalModel)
	{
		originalModel->draw(cmd);
	}
}

void DualModel::drawSimplifiedModel(vk::CommandBuffer cmd) const
{
	if (simplifiedModel)
	{
		simplifiedModel->draw(cmd);
	}
}

void DualModel::simplifyModel(const std::vector<MeshData>& newMeshesData)
{
	if (newMeshesData.empty())
	{
		throw std::runtime_error("New meshes are empty.");
	}

	if (!simplifiedModel)
	{
		throw std::runtime_error("Simplified model is not initialized.");
	}

	try {
		
		simplifiedModel = std::make_unique<Model>(dev, newMeshesData);
		wasSimplified = true;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to apply simplification: " << e.what() << std::endl;
		// fallback when something goes wrong
		createSimplifiedCopy();
		throw;
	}
}

void DualModel::revertSimplification()
{
	createSimplifiedCopy();
	wasSimplified = false;
}

/* End of the DualModel.cpp file */