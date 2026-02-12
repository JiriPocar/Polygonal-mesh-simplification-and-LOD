#pragma once
#include "Model.hpp"
#include <memory>

class CommandManager;

class DualModel {
public:
	DualModel(Device& device, CommandManager& cmd, const std::string& modelPath);
	~DualModel() = default;

	Model& getOriginalModel() const { return *originalModel; }
	Model& getSimplifiedModel() const { return *simplifiedModel; }

	void drawOriginalModel(vk::CommandBuffer cmd) const;
	void drawSimplifiedModel(vk::CommandBuffer cmd) const;

	void revertSimplification();
	void simplifyModel(const std::vector<Vertex>& newVertices, const std::vector<uint32_t>& newIndices);
	bool wasModelSimplified() const { return wasSimplified; }

private:
	void createSimplifiedCopy();

	Device& dev;
	std::unique_ptr<Model> originalModel;
	std::unique_ptr<Model> simplifiedModel;
	bool wasSimplified = false;
};