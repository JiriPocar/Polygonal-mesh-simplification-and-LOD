#include "SpiralScene.hpp"
#include <cmath>
#include <iostream>
#include <chrono>

SpiralScene::SpiralScene(Device& dev, CommandManager& cmd, const std::string& modelPath, UniformBuffer& uniformBuffer)
	: dev(dev), uniformBuffer(uniformBuffer), modelPath(modelPath)
{
	generateSpiralPositions();
	generateLODVersions(cmd);
	createInstanceBuffer();
	createIndirectBuffer();
	createLODInstanceBuffer();
	std::cout << "SpiralScene created" << std::endl;
}

void SpiralScene::generateSpiralPositions()
{
	positions.clear();
	positions.resize(MAX_INSTANCE_COUNT);
	instanceData.resize(MAX_INSTANCE_COUNT);

	float spacing = 2.0f;
	int numArms = 5;
	float minRadius = 10.0f;
	float coneFactor = 0.5f;
	float twistSpeed = 0.02f;

	float totalLength = config.instanceCount * spacing;

	for (int i = 0; i < config.instanceCount; i++)
	{
		float linearPos = (float)i * spacing;
		float distance = fmod(linearPos, totalLength);
		float currentZ = -distance;
		float baseArmAngle = (i % numArms) * (6.28318f / numArms);
		float twistAngle = abs(currentZ) * twistSpeed;
		float finalAngle = baseArmAngle + twistAngle;
		float currentRadius = minRadius + (abs(currentZ) * coneFactor);

		glm::vec3 pos = glm::vec3(
			currentRadius * cos(finalAngle),
			currentRadius * sin(finalAngle),
			currentZ
		);

		positions.push_back(pos);
		instanceData[i].pos = pos;
		instanceData[i].modelTypeIndex = 0;
		instanceData[i].lodLevel = 0;
	}
}

void SpiralScene::generateLODVersions(CommandManager& cmd)
{
	ModelLODSet lodSet;
	lodSet.name = modelPath;

	auto originalModel = std::make_unique<Model>(dev, cmd, modelPath);

	simplificator.setCurrentAlgorithm(Algorithm::QEM);
	simplificator.options.checkConnectivity = true;
	simplificator.options.checkFaceFlipping = true;
	simplificator.options.enableMerging = true;
	simplificator.options.mergeCloseVertivesPos = true;

	auto result1 = simplificator.simplify(*originalModel, config.lodPercentageSimplification1); // 75% faces
	lodSet.lod1 = std::make_unique<Model>(dev, result1.meshesData);

	auto result2 = simplificator.simplify(*originalModel, config.lodPercentageSimplification2); // 50% faces
	lodSet.lod2 = std::make_unique<Model>(dev, result2.meshesData);

	auto result3 = simplificator.simplify(*originalModel, config.lodPercentageSimplification3); // 25% faces
	lodSet.lod3 = std::make_unique<Model>(dev, result3.meshesData);

	if (config.lodPercentageSimplification0 >= 0.99f)
	{
		lodSet.lod0 = std::move(originalModel);
	}
	else
	{
		auto result0 = simplificator.simplify(*originalModel, config.lodPercentageSimplification0);
		lodSet.lod0 = std::make_unique<Model>(dev, result0.meshesData);
		lodSet.lod0->setTexture(originalModel->releaseTexture());
	}


	modelLODSets.push_back(std::move(lodSet));
}

void SpiralScene::rebuildLODs(CommandManager& cmd)
{
	// wait for device to be idle
	dev.operator*().waitIdle();

	// clear existing LODs and generate new ones based on current model path
	modelLODSets.clear();
	generateLODVersions(cmd);

	vk::CommandBuffer singleCmd = cmd.beginSingleTimeCommands();
	resetIndirectBuffer(singleCmd, 0);
	resetIndirectBuffer(singleCmd, 1);
	cmd.endSingleTimeCommands(singleCmd);
}

void SpiralScene::createInstanceBuffer()
{
	int frameCount = 2;
	instanceBuffers.resize(frameCount);
	vk::DeviceSize bufferSize = sizeof(SpiralInstanceData) * instanceData.size();

	for (int i = 0; i < frameCount; i++)
	{
		instanceBuffers[i] = std::make_unique<Buffer>(
			dev,
			bufferSize,
			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			VMA_ALLOCATION_CREATE_MAPPED_BIT
		);
	}
}

void SpiralScene::createIndirectBuffer()
{
	int frameCount = 2;
	indirectBuffers.resize(frameCount);
	vk::DeviceSize bufferSize = sizeof(DrawIndexedIndirectCommand) * 4; // 4 LOD levels

	for (int i = 0; i < frameCount; i++)
	{
		indirectBuffers[i] = std::make_unique<Buffer>(
			dev,
			bufferSize,
			vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
			VMA_MEMORY_USAGE_GPU_ONLY
		);
	}
}

void SpiralScene::createLODInstanceBuffer()
{
	int frameCount = 2;
	LODInstanceBuffers.resize(frameCount);
	vk::DeviceSize bufferSize = sizeof(SpiralInstanceData) * MAX_INSTANCE_COUNT * 4;

	for (int i = 0; i < frameCount; i++)
	{
		LODInstanceBuffers[i] = std::make_unique<Buffer>(
			dev,
			bufferSize,
			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
			VMA_MEMORY_USAGE_GPU_ONLY
		);
	}
}

void SpiralScene::updateLODs(const glm::vec3& cameraPos, uint32_t currentFrame, bool useGPULOD, bool useGPUSpiral)
{
	if (useGPULOD)
	{
		if (!useGPUSpiral)
		{
			for (uint32_t i = 0; i < config.instanceCount; i++) {
				instanceData[i].pos = positions[i];
				instanceData[i].lodLevel = 0;
			}
			instanceBuffers[currentFrame]->copyData(instanceData.data(), sizeof(SpiralInstanceData) * config.instanceCount);
		}
		
		return;
	}
	else
	{
		updateInstancesCPU(cameraPos, currentFrame);
	}
}

void SpiralScene::updateInstancesCPU(const glm::vec3& cameraPos, uint32_t currentFrame)
{
	// we are gonna want to have instances sorted by LOD level here for better GPU performance
	// using compute sort, we can achieve time complexity of O(n)
	// reference: https://www.geeksforgeeks.org/dsa/counting-sort/
	lodCounts = { 0, 0, 0, 0 };
	std::vector<uint8_t> instanceLODs(config.instanceCount);

	float squaredLODdist0 = config.lodDist0 * config.lodDist0;
	float squaredLODdist1 = config.lodDist1 * config.lodDist1;
	float squaredLODdist2 = config.lodDist2 * config.lodDist2;

	// histogram
	for (uint32_t i = 0; i < config.instanceCount; i++)
	{
		// expensive distance calculation, we only need relative distances
		//float dist = glm::distance(cameraPos, positions[i]);
		float dist = glm::length2(cameraPos - positions[i]); // <- can use instead
		uint8_t lod = 0;

		if (config.enableLOD)
		{
			lod = 3; // default to lowest LOD
			if (dist < squaredLODdist0)
			{
				lod = 0;
			}
			else if (dist < squaredLODdist1)
			{
				lod = 1;
			}
			else if (dist < squaredLODdist2)
			{
				lod = 2;
			}
		}

		instanceLODs[i] = lod;
		lodCounts[lod]++;
	}

	// prefix sum to get offsets
	lodOffsets[0] = 0;
	lodOffsets[1] = lodCounts[0];
	lodOffsets[2] = lodOffsets[1] + lodCounts[1];
	lodOffsets[3] = lodOffsets[2] + lodCounts[2];

	std::array<uint32_t, 4> currentOffsets = lodOffsets;

	// sorting to buffer in LOD order
	for (uint32_t i = 0; i < config.instanceCount; i++)
	{
		uint8_t lod = instanceLODs[i];
		uint32_t targetIdx = currentOffsets[lod]++;

		instanceData[targetIdx].pos = positions[i];
		instanceData[targetIdx].modelTypeIndex = 0; // single model type for now
		instanceData[targetIdx].lodLevel = lod;
	}

	instanceBuffers[currentFrame]->copyData(instanceData.data(), sizeof(SpiralInstanceData) * config.instanceCount);
}

void SpiralScene::updateSpiralPositions(float deltaTime, bool useGPUSpiral)
{
	animationTime += deltaTime * config.speed;

	if (useGPUSpiral) return;

	float totalLength = config.instanceCount * config.spacing;

	for (uint32_t i = 0; i < config.instanceCount; i++)
	{
		// depth
		// where would instance be on a straight infinite line
		float linearPos = (i * config.spacing) + animationTime;
		// infinite effect via fmod
		float distance = fmod(linearPos, totalLength);
		// flip to negative z
		float currentZ = -distance;

		// base angle of the arm
		// (i % numArms) ensures that instance 0 goes to arm 0, instance 1 to arm 1...
		float baseArmAngle = (i % config.numArms) * (6.28318f / config.numArms);

		// angle twist based on depth, deeper means more twist
		float twistAngle = abs(currentZ) * config.twistSpeed;

		// final angle, sum of base arm angle and twist
		float finalAngle = baseArmAngle + twistAngle;

		// radius
		// starts at minRadius and grows linearly with depth
		float currentRadius = config.minRadius + (abs(currentZ) * config.coneFactor);

		// polar to cartesian conversion
		glm::vec3 pos = glm::vec3(
			currentRadius * cos(finalAngle),
			currentRadius * sin(finalAngle),
			currentZ
		);

		positions[i] = pos;
	}

}

uint32_t SpiralScene::calculateCurrentDrawnTriangles(const glm::vec3& cameraPos)
{
	// DO NOT CALL THIS FUNCTION EVERY FRAME SINCE ITS COSTLY
	// USE FOR:
	//		a) for benchmark tasks (collect only on start)
	//		b) UI display (display only two times per sec)

	uint32_t totalTriangles = 0;

	// if LOD is disabled, then all instances are drwan with LOD0
	if (!config.enableLOD)
	{
		uint32_t lod0Faces = modelLODSets[0].getLOD(0).getIndexCount() / 3;
		return config.instanceCount * lod0Faces;
	}

	// if scene was computed by CPU, we know exact numbers
	if (lodCounts[0] > 0 || lodCounts[1] > 0 || lodCounts[2] > 0 || lodCounts[3] > 0)
	{
		for (int i = 0; i < 4; i++)
		{
			totalTriangles += lodCounts[i] * (modelLODSets[0].getLOD(i).getIndexCount() / 3);
		}

		return totalTriangles;
	}

	// if scene was computed by GPU, we need either need to get data from GPU (this is costly)
	// or we can compute an estimation from "time freeze" instance positions
	float sqDist0 = config.lodDist0 * config.lodDist0;
	float sqDist1 = config.lodDist1 * config.lodDist1;
	float sqDist2 = config.lodDist2 * config.lodDist2;

	std::array<uint32_t, 4> timefreezeCounts = { 0, 0, 0, 0 };

	for (uint32_t i = 0; i < config.instanceCount; i++)
	{
		float dist = glm::length2(cameraPos - positions[i]);

		uint8_t lod = 3;
		if (dist < sqDist0)
		{
			lod = 0;
		}
		else if (dist < sqDist1)
		{
			lod = 1;
		}
		else if (dist < sqDist2)
		{
			lod = 2;
		}

		timefreezeCounts[lod]++;
	}

	for (int i = 0; i < 4; i++)
	{
		totalTriangles += timefreezeCounts[i] * (modelLODSets[0].getLOD(i).getIndexCount() / 3);
	}

	return totalTriangles;
}

void SpiralScene::resetIndirectBuffer(vk::CommandBuffer cmd, uint32_t currentFrame)
{
	// prepare four commands for four LOD levels, instance count will be updated by compute shader after culling
	DrawIndexedIndirectCommand cmds[4];

	for (int i = 0; i < 4; i++)
	{
		// get exact index count for each LOD level to ensure correct draw calls
		Model& lodModel = modelLODSets[0].getLOD(i);

		cmds[i].indexCount = static_cast<uint32_t>(lodModel.getIndexCount());
		cmds[i].instanceCount = 0; // will be updated by compute shader
		cmds[i].firstIndex = 0;
		cmds[i].vertexOffset = 0;

		// base offset for each LOD level
		cmds[i].firstInstance = i * MAX_INSTANCE_COUNT;
	}

	// copy the commands to the indirect buffer
	cmd.updateBuffer(indirectBuffers[currentFrame]->getBuffer(), 0, sizeof(cmds), cmds);
}

void SpiralScene::addModelType(const std::string& modelPath, CommandManager& cmd)
{
	generateLODVersions(cmd);
}
