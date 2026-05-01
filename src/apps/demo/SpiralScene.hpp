#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <vector>
#include <memory>

#include "common/core/Device.hpp"
#include "common/resources/Model.hpp"
#include "common/simplification/Simplificator.hpp"
#include "common/rendering/CommandManager.hpp"
#include "common/rendering/UniformBuffer.hpp"

// matches the structure in the shader for instance data
struct SpiralInstanceData {
	glm::vec3 pos;		// 12 bytes
	uint32_t lodLevel;	// 4 bytes
};

// matches the structure in the shader for indirect draw commands
struct DrawIndexedIndirectCommand {
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t firstInstance;
};

// push constants used in the compute shader
struct ComputePushConstants {
	uint32_t totalInstances;
	float lodDist0;
	float lodDist1;
	float lodDist2;
	uint32_t computeSpiral;
	float spacing;
	int numArms;
	float minRadius;
	float coneFactor;
	float twistSpeed;
	float animationTime;
	uint32_t enableLOD;
};

struct SpiralConfig {
	uint32_t instanceCount = 10000;
	float spacing = 2.0f;
	float speed = 30.0f;
	int numArms = 5;
	float minRadius = 10.0f;
	float coneFactor = 0.5f;
	float twistSpeed = 0.02f;

	bool enableLOD = true;
	float lodDist0 = 400.0f;
	float lodDist1 = 1200.0f;
	float lodDist2 = 3000.0f;

	float lodPercentageSimplification0 = 1.0f;
	float lodPercentageSimplification1 = 0.5f;
	float lodPercentageSimplification2 = 0.25f;
	float lodPercentageSimplification3 = 0.10f;
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

	vk::Buffer getInstanceBuffer(uint32_t currentFrame) const { return instanceBuffers[currentFrame]->getBuffer(); }
	uint32_t getMaxInstanceCount() const { return maxInstanceCount; }
	void setMaxInstanceCount(uint32_t newMax) { maxInstanceCount = newMax; }
	uint32_t getLODCount(uint32_t lodLevel) const { return lodCounts[lodLevel]; }
	uint32_t getLODOffset(uint32_t lodLevel) const { return lodOffsets[lodLevel]; }
	ModelLODSet& getModelLODSet() { return modelLODSet; }
	const std::vector<SpiralInstanceData>& getInstanceData() const { return instanceData; }
	vk::Buffer getIndirectBuffer(uint32_t currentFrame) const { return indirectBuffers[currentFrame]->getBuffer(); }
	UniformBuffer& getUniformBuffer() const { return uniformBuffer; }
	vk::Buffer getLODInstanceBuffer(uint32_t currentFrame) const { return LODInstanceBuffers[currentFrame]->getBuffer(); }
	float getAnimationTime() const { return animationTime; }

	uint64_t calculateCurrentDrawnTriangles(const glm::vec3& cameraPos, std::array<uint32_t, 4>& outLodCounts);

	void resetIndirectBuffer(vk::CommandBuffer cmd, uint32_t currentFrame);
	void reallocBuffers(uint32_t newMaxCount);
	void resetAnimation() { animationTime = 0.0f; }
	SpiralConfig config;

private:
	void initSpiralPositions();
	void generateLODVersions(CommandManager& cmd);
	void createInstanceBuffer();
	void updateInstancesCPU(const glm::vec3& cameraPos, uint32_t currentFrame);

	void createIndirectBuffer();
	void createLODInstanceBuffer();

	uint32_t maxInstanceCount = 100000;

	Device& dev;
	UniformBuffer& uniformBuffer;
	std::string modelPath;

	std::vector<glm::vec3> positions;
	std::vector<SpiralInstanceData> instanceData;
	ModelLODSet modelLODSet;

	std::vector<std::unique_ptr<Buffer>> indirectBuffers;
	std::vector<std::unique_ptr<Buffer>> LODInstanceBuffers;
	std::vector<std::unique_ptr<Buffer>> instanceBuffers;

	std::array<uint32_t, 4> lodCounts = { 0, 0, 0, 0 };
	std::array<uint32_t, 4> lodOffsets = { 0, 0, 0, 0 };

	Simplificator simplificator;

	float animationTime = 0.0f;
};
