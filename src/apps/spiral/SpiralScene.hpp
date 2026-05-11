/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SpiralScene.hpp
 * @brief Implementation of the SpiralScene class for the Spiral application.
 *
 * This file implements the SpiralScene class, which is responsible for managing
 * the data and logic of the spiral scene in the Spiral application. It handles
 * the generation of spiral instance positions, LOD management, and buffer creation
 * for both CPU and GPU-based rendering approaches.
 */

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

	/**
	* @brief Based on the computational mode (CPU x GPU) updates the instance data with correct LOD levels for each instance.
	* 
	* @param cameraPos The position of the camera to determine LOD levels based on distance
	* @param currentFrame The current frame index for double buffering
	* @param useGPULOD Where to do LOD selection
	* @param useGPUSpiral Where to compute spiral positions for instances
	*/
	void updateLODs(const glm::vec3& cameraPos, uint32_t currentFrame, bool useGPULOD, bool useGPUSpiral);

	/**
	* @brief Updates the positions of the spiral instances based on the current animation time and configuration parameters.
	* 
	* @param deltaTime Time delta since the last update, used to advance the animation
	* @param useGPUSpiral Where to compute spiral positions for instances
	*/
	void updateSpiralPositions(float deltaTime, bool useGPUSpiral);

	/**
	* @brief Rebuilds the LOD versions of the model based on the current simplification configuration parameters.
	* 
	* @param cmd For recording a single time command to reset indirect buffers after LOD generation
	*/
	void rebuildLODs(CommandManager& cmd);

	// getters and setters
	vk::Buffer getInstanceBuffer(uint32_t currentFrame) const { return gpuInstanceBuffers[currentFrame]->getBuffer(); }
	uint32_t getMaxInstanceCount() const { return maxInstanceCount; }
	void setMaxInstanceCount(uint32_t newMax) { maxInstanceCount = newMax; }
	uint32_t getLODCount(uint32_t lodLevel) const { return lodCounts[lodLevel]; }
	uint32_t getLODOffset(uint32_t lodLevel) const { return lodOffsets[lodLevel]; }
	ModelLODSet& getModelLODSet() { return modelLODSet; }
	const std::vector<SpiralInstanceData>& getInstanceData() const { return instanceData; }
	vk::Buffer getIndirectBuffer(uint32_t currentFrame) const { return indirectBuffers[currentFrame]->getBuffer(); }
	UniformBuffer& getUniformBuffer() const { return uniformBuffer; }
	vk::Buffer getOutputInstanceBuffer(uint32_t currentFrame) const { return outputInstanceBuffers[currentFrame]->getBuffer(); }
	float getAnimationTime() const { return animationTime; }
	vk::Buffer getVRAMVertexBuffer(uint32_t lodLevel) const { return vramVertexBuffers[lodLevel]->getBuffer(); }
	vk::Buffer getVRAMIndexBuffer(uint32_t lodLevel) const { return vramIndexBuffers[lodLevel]->getBuffer(); }

	/**
	* @brief Calculates the total number of triangles currently being drawn based on the camera position and LOD counts.
	* 
	* @param cameraPos The position of the camera to determine LOD levels based on distance
	* @param outLodCounts Output array to store the count of instances for each LOD level
	* 
	* @return The total number of triangles currently being drawn in the scene.
	*/
	uint64_t calculateCurrentDrawnTriangles(const glm::vec3& cameraPos, std::array<uint32_t, 4>& outLodCounts);

	/**
	* @brief Resets the indirect draw command buffer before compute shader dispatch.
	* 
	* @param cmd The command buffer to record into
	* @param currentFrame The current frame index for double buffering
	*/
	void resetIndirectBuffer(vk::CommandBuffer cmd, uint32_t currentFrame);

	/**
	* @brief Records commands to transfer instance data from the staging buffer to the GPU buffer for the current frame.
	* 
	* @param cmd The command buffer to record into
	* @param currentFrame The current frame index for double buffering
	*/
	void recordInstanceTransfer(vk::CommandBuffer cmd, uint32_t currentFrame);

	/**
	* @brief Reallocates buffers of the scene after new max instance count is set.
	* 
	* @param newMaxCount The new maximum instance count to allocate buffers for
	*/
	void reallocBuffers(uint32_t newMaxCount);

	// resets animation time
	void resetAnimation() { animationTime = 0.0f; }
	SpiralConfig config;

private:
	/**
	* @brief Initializes the spiral positions for all instances.
	*/
	void initSpiralPositions();

	/**
	* @brief Generates LOD versions of the model using the simplificator.
	* 
	* @param cmd Command manager for textures processing (transition layouts).
	*/
	void generateLODVersions(CommandManager& cmd);

	/**
	* @brief Creates the staging buffer for instance data that will be updated on CPU and copied to GPU buffer.
	*/
	void createStagingInstanceBuffer();

	/**
	* @brief Creates the GPU buffer for instance data that will be used for rendering and possibly updated by compute shader.
	*/
	void createGPUInstanceBuffer();

	/**
	* @brief Uploads the vertex and index data of all LOD levels to GPU buffers for rendering.
	* 
	* @param cmd Command manager for recording copy commands to transfer vertex and index data to GPU buffers.
	*/
	void uploadGeometryToVRAM(CommandManager& cmd);

	/**
	* @brief Creates the indirect draw command buffer for GPU-driven rendering.
	*/
	void createIndirectBuffer();

	/**
	* @brief Creates the output instance buffer that the compute shader will write to.
	*/
	void createOutputInstanceBuffer();

	/**
	* @brief Updates the instances on CPU. Sorts the instance data into LOD order and copies it to the instance buffer.
	* 
	* @param cameraPos The position of the camera to determine LOD levels based on distance
	* @param currentFrame The current frame index for double buffering
	*/
	void updateInstancesCPU(const glm::vec3& cameraPos, uint32_t currentFrame);

	// this is a dynamic value that can grow when set higher
	uint32_t maxInstanceCount = 100000;

	// misc
	Device& dev;
	UniformBuffer& uniformBuffer;
	std::string modelPath;

	// spiral and LOD specific data
	std::vector<glm::vec3> positions;
	std::vector<SpiralInstanceData> instanceData;
	ModelLODSet modelLODSet;
	std::array<uint32_t, 4> lodCounts = { 0, 0, 0, 0 };
	std::array<uint32_t, 4> lodOffsets = { 0, 0, 0, 0 };
	Simplificator simplificator;
	float animationTime = 0.0f;

	// used buffers
	std::vector<std::unique_ptr<Buffer>> indirectBuffers;
	std::vector<std::unique_ptr<Buffer>> outputInstanceBuffers;
	std::vector<std::unique_ptr<Buffer>> stagingInstanceBuffers;
	std::vector<std::unique_ptr<Buffer>> gpuInstanceBuffers;
	std::array<std::unique_ptr<Buffer>, 4> vramVertexBuffers;
	std::array<std::unique_ptr<Buffer>, 4> vramIndexBuffers;
};

/* End of the SpiralScene.hpp file */