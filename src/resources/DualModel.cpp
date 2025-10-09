#include "DualModel.hpp"

DualModel::DualModel(const Device& device, const std::string& modelPath)
{
	originalModel = std::make_unique<Model>(device, modelPath);
	createSimplifiedCopy();
}

void DualModel::createSimplifiedCopy()
{

}

void DualModel::revertSimplification()
{
	createSimplifiedCopy();
}