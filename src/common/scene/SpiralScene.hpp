#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "core/Device.hpp"
#include "resources/Model.hpp"
#include "../simplification/Simplificator.hpp"

const uint32_t MAX_INSTANCE_COUNT = 1000000;

struct SpiralInstanceData {
	glm::vec3 pos;
	float padding1;
	uint32_t modelTypeIndex;
	uint32_t lodLevel;
	glm::vec2 padding2;
};

struct SpiralConfig {
	uint32_t instanceCount = 10000;
	float spacing = 2.0f;
	float speed = 30.0f;
	int numArms = 5;
	float minRadius = 10.0f;
	float coneFactor = 0.5f;
	float twistSpeed = 0.02f;
	float armSpread = 0.6f;
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
	SpiralScene(Device& dev, const std::string& modelPath);
	~SpiralScene() = default;

	void updateLODs(const glm::vec3& cameraPos, uint32_t currentFrame);
	void updateSpiralPositions(float deltaTime);

	vk::Buffer getInstanceBuffer(uint32_t currentFrame) const { return instanceBuffers[currentFrame]; }
	uint32_t getMaxInstanceCount() const { return MAX_INSTANCE_COUNT; }
	uint32_t getLODCount(uint32_t lodLevel) const { return lodCounts[lodLevel]; }
	uint32_t getLODOffset(uint32_t lodLevel) const { return lodOffsets[lodLevel]; }
	ModelLODSet& getModelLODSet(uint32_t index = 0) { return modelLODSets[index]; }
	const std::vector<SpiralInstanceData>& getInstanceData() const { return instanceData; }
	uint32_t getModelTypeCount() const { return static_cast<uint32_t>(modelLODSets.size()); }

	void addModelType(const std::string& modelPath);
	void resetAnimation() { animationTime = 0.0f; }
	SpiralConfig config;

private:
	void generateSpiralPositions();
	void generateLODVersions(const std::string& modelPath);
	void createInstanceBuffer();
	void updateInstancesCPU(const glm::vec3& cameraPos, uint32_t currentFrame);

	Device& dev;

	std::vector<glm::vec3> positions;
	std::vector<SpiralInstanceData> instanceData;
	std::vector<ModelLODSet> modelLODSets;

	std::vector<vk::Buffer> instanceBuffers;
	std::vector<vk::DeviceMemory> instanceBufferMemory;
	std::vector<void*> mappedMemory;

	float lodDistances[4] = { 400.0f, 1200.0f, 3000.0f, 8000.0f};
	float animationTime = 0.0f;

	std::array<uint32_t, 4> lodCounts = { 0, 0, 0, 0 };
	std::array<uint32_t, 4> lodOffsets = { 0, 0, 0, 0 };

	Simplificator simplificator;
};
