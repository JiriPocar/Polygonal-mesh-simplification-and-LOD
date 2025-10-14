#include "DualModel.hpp"

DualModel::DualModel(const Device& device, const std::string& modelPath)
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
}

void DualModel::revertSimplification()
{
	createSimplifiedCopy();
}

