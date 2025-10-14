#pragma once
#include "Model.hpp"
#include <memory>

class DualModel {
public:
	DualModel(const Device& device, const std::string& modelPath);
	~DualModel() = default;

	Model& getOriginalModel() const { return *originalModel; }
	Model& getSimplifiedModel() const { return *simplifiedModel; }

	void drawOriginalModel(vk::CommandBuffer cmd) const;
	void drawSimplifiedModel(vk::CommandBuffer cmd) const;

	void revertSimplification();
	void simplifyModel(const std::vector<Vertex>& newVertices, const std::vector<uint32_t>& newIndices);

private:
	void createSimplifiedCopy();

	std::unique_ptr<Model> originalModel;
	std::unique_ptr<Model> simplifiedModel;
};