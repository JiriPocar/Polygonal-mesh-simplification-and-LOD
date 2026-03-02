#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "core/Device.hpp"
#include "resources/Model.hpp"
#include "../simplification/Simplificator.hpp"
#include "../rendering/CommandManager.hpp"
#include "../rendering/UniformBuffer.hpp"

const uint32_t MAX_INSTANCE_COUNT = 1000000;

struct SpiralInstanceData {
	glm::vec3 pos;
	float padding1;
	uint32_t modelTypeIndex;
	uint32_t lodLevel;
	glm::vec2 padding2;
};

struct DrawIndexedIndirectCommand {
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t firstInstance;
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

	bool enableLOD = true;
	float lodDist0 = 400.0f;
	float lodDist1 = 1200.0f;
	float lodDist2 = 3000.0f;
	float lodDist3 = 8000.0f;

	float lodPercentageSimplification0 = 1.0f;
	float lodPercentageSimplification1 = 0.75f;
	float lodPercentageSimplification2 = 0.5f;
	float lodPercentageSimplification3 = 0.25f;
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
	SpiralScene(Device& dev, CommandManager& cmd, const std::string& modelPath, UniformBuffer& uniformBuffer);
	~SpiralScene() = default;

	void updateLODs(const glm::vec3& cameraPos, uint32_t currentFrame, bool useGPULOD, bool useGPUSpiral);
	void updateSpiralPositions(float deltaTime, bool useGPUSpiral);
	void rebuildLODs(CommandManager& cmd);

	vk::Buffer getInstanceBuffer(uint32_t currentFrame) const { return instanceBuffers[currentFrame]; }
	uint32_t getMaxInstanceCount() const { return MAX_INSTANCE_COUNT; }
	uint32_t getLODCount(uint32_t lodLevel) const { return lodCounts[lodLevel]; }
	uint32_t getLODOffset(uint32_t lodLevel) const { return lodOffsets[lodLevel]; }
	ModelLODSet& getModelLODSet(uint32_t index = 0) { return modelLODSets[index]; }
	const std::vector<SpiralInstanceData>& getInstanceData() const { return instanceData; }
	vk::Buffer getIndirectBuffer(uint32_t currentFrame) const { return indirectBuffers[currentFrame]; }
	UniformBuffer& getUniformBuffer() const { return uniformBuffer; }
	vk::Buffer getLODInstanceBuffer(uint32_t currentFrame) const { return LODInstanceBuffers[currentFrame]; }
	uint32_t getModelTypeCount() const { return static_cast<uint32_t>(modelLODSets.size()); }
	float getAnimationTime() const { return animationTime; }

	void resetIndirectBuffer(uint32_t currentFrame);

	void addModelType(const std::string& modelPath, CommandManager& cmd);
	void resetAnimation() { animationTime = 0.0f; }
	SpiralConfig config;

private:
	void generateSpiralPositions();
	void generateLODVersions(CommandManager& cmd);
	void createInstanceBuffer();
	void updateInstancesCPU(const glm::vec3& cameraPos, uint32_t currentFrame);

	void createIndirectBuffer();
	void createLODInstanceBuffer();
	std::vector<vk::Buffer> indirectBuffers;
	std::vector<vk::DeviceMemory> indirectBufferMemory;
	std::vector<void*> mappedIndirectMemory;
	std::vector<vk::Buffer> LODInstanceBuffers;
	std::vector<vk::DeviceMemory> LODInstanceBufferMemory;

	Device& dev;
	UniformBuffer& uniformBuffer;
	std::string modelPath;

	std::vector<glm::vec3> positions;
	std::vector<SpiralInstanceData> instanceData;
	std::vector<ModelLODSet> modelLODSets;

	std::vector<vk::Buffer> instanceBuffers;
	std::vector<vk::DeviceMemory> instanceBufferMemory;
	std::vector<void*> mappedMemory;

	std::array<uint32_t, 4> lodCounts = { 0, 0, 0, 0 };
	std::array<uint32_t, 4> lodOffsets = { 0, 0, 0, 0 };

	Simplificator simplificator;

	float animationTime = 0.0f;
};
