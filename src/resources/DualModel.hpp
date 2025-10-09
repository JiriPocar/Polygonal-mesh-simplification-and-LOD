#pragma once
#include "Model.hpp"
#include <memory>

class DualModel {
public:
	DualModel(const Device& device, const std::string& modelPath);
	~DualModel() = default;

	const Model& getOriginalModel() const { return *originalModel; }
	const Model& getSimplifiedModel() const { return *simplifiedModel; }

	void drawOriginalModel(vk::CommandBuffer cmd) const;
	void drawSimplifiedModel(vk::CommandBuffer cmd) const;

	void revertSimplification();

private:
	void createSimplifiedCopy();

	std::unique_ptr<Model> originalModel;
	std::unique_ptr<Model> simplifiedModel;
};