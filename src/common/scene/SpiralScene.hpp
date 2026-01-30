#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "core/Device.hpp"
#include "resources/Model.hpp"
#include "../simplification/Simplificator.hpp"

struct SpiralInstanceData {
	glm::vec3 pos;
	float padding1;
	uint32_t modelTypeIndex;
	uint32_t lodLevel;
	glm::vec2 padding2;
};

struct ModelLODSet {
	std::string name;
	std::unique_ptr<Model> lod0;
	std::unique_ptr<Model> lod1;
	std::unique_ptr<Model> lod2;
	std::unique_ptr<Model> lod3;

	Model& getLOD(uint32_t level)
	{
		switch (level)
		{
			case 0:
				return *lod0;
			case 1:
				return *lod1;
			case 2:
				return *lod2;
			case 3:
				return *lod3;
			default:
				return *lod0;
		}
	}
};

class SpiralScene {
public:
	SpiralScene(Device& dev, uint32_t instanceCount, const std::string& modelPath);
	~SpiralScene() = default;

	void updateLODs(const glm::vec3& cameraPos);
	void updatePositions(float deltaTime);

	vk::Buffer getInstanceBuffer() const { return instanceBuffer; }
	uint32_t getInstanceCount() const { return instanceCount; }

	ModelLODSet& getModelLODSet(uint32_t index = 0) { return modelLODSets[index]; }
	const std::vector<SpiralInstanceData>& getInstanceData() const { return instanceData; }

	void addModelType(const std::string& modelPath);
	uint32_t getModelTypeCount() const { return static_cast<uint32_t>(modelLODSets.size()); }

private:
	void generateSpiralPositions();
	void generateLODVersions(const std::string& modelPath);
	void createInstanceBuffer();
	void updateInstancesCPU(const glm::vec3& cameraPos);

	Device& dev;
	uint32_t instanceCount;

	std::vector<glm::vec3> positions;
	std::vector<SpiralInstanceData> instanceData;
	std::vector<ModelLODSet> modelLODSets;

	vk::Buffer instanceBuffer;
	vk::DeviceMemory instanceBufferMemory;
	void* mappedMemory;

	float lodDistances[4] = { 30.0f, 70.0f, 150.0f, 300.0f};
	float animationTime = 0.0f;
	float animationSpeed = 1.0f;

	Simplificator simplificator;
};
