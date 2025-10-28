#include "DualModel.hpp"
#include <iostream>

DualModel::DualModel(const Device& device, const std::string& modelPath) : dev(const_cast<Device&>(device))
{
	originalModel = std::make_unique<Model>(device, modelPath);
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

void DualModel::simplifyModel(const std::vector<Vertex>& newVertices, const std::vector<uint32_t>& newIndices)
{
	if (newVertices.empty() || newIndices.empty())
	{
		throw std::runtime_error("New vertices or indices are empty.");
	}

	if (!simplifiedModel)
	{
		throw std::runtime_error("Simplified model is not initialized.");
	}

	try {
		
		simplifiedModel = std::make_unique<Model>(dev, newVertices, newIndices);
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

