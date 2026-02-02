#include "SpiralScene.hpp"
#include <cmath>
#include <iostream>
#include <chrono>

SpiralScene::SpiralScene(Device& dev, const std::string& modelPath)
	: dev(dev)
{
	generateSpiralPositions();
	generateLODVersions(modelPath);
	createInstanceBuffer();
	std::cout << "SpiralScene created" << std::endl;
}

void SpiralScene::generateSpiralPositions()
{
	positions.clear();
	positions.reserve(MAX_INSTANCE_COUNT);
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

void SpiralScene::generateLODVersions(const std::string& modelPath)
{
	ModelLODSet lodSet;
	lodSet.name = modelPath;

	lodSet.lod0 = std::make_unique<Model>(dev, modelPath); // Original model

	simplificator.setCurrentAlgorithm(Algorithm::QEM);
	auto result1 = simplificator.simplify(*lodSet.lod0, 0.75f); // 75% faces
	lodSet.lod1 = std::make_unique<Model>(dev, result1.vertices, result1.indices);
	auto result2 = simplificator.simplify(*lodSet.lod0, 0.50f); // 50% faces
	lodSet.lod2 = std::make_unique<Model>(dev, result2.vertices, result2.indices);
	auto result3 = simplificator.simplify(*lodSet.lod0, 0.25f); // 25% faces
	lodSet.lod3 = std::make_unique<Model>(dev, result3.vertices, result3.indices);

	modelLODSets.push_back(std::move(lodSet));
}

void SpiralScene::createInstanceBuffer()
{
	int frameCount = 2;
	instanceBuffers.resize(frameCount);
	instanceBufferMemory.resize(frameCount);
	mappedMemory.resize(frameCount);

	vk::DeviceSize bufferSize = sizeof(SpiralInstanceData) * instanceData.size();

	for (int i = 0; i < frameCount; i++)
	{
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.size = bufferSize;
		bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		instanceBuffers[i] = dev.operator*().createBuffer(bufferInfo);

		vk::MemoryRequirements memReq = dev.operator*().getBufferMemoryRequirements(instanceBuffers[i]);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = dev.findMemoryType(memReq.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		instanceBufferMemory[i] = dev.operator*().allocateMemory(allocInfo);
		dev.operator*().bindBufferMemory(instanceBuffers[i], instanceBufferMemory[i], 0);

		mappedMemory[i] = dev.operator*().mapMemory(
			instanceBufferMemory[i],
			0,
			VK_WHOLE_SIZE,
			vk::MemoryMapFlags{}
		);
	}
	
}

void SpiralScene::updateLODs(const glm::vec3& cameraPos, uint32_t currentFrame)
{
	updateInstancesCPU(cameraPos, currentFrame);
}

void SpiralScene::updateInstancesCPU(const glm::vec3& cameraPos, uint32_t currentFrame)
{
	// we are gonna want to have instances sorted by LOD level here for better GPU performance
	// using compute sort, we can achieve time complexity of O(n)
	// reference: https://www.geeksforgeeks.org/dsa/counting-sort/
	lodCounts = { 0, 0, 0, 0 };
	std::vector<uint8_t> instanceLODs(config.instanceCount);

	// histogram
	for (uint32_t i = 0; i < config.instanceCount; i++)
	{
		float dist = glm::distance(cameraPos, positions[i]);
		uint8_t lod = 0;

		if (dist < lodDistances[0])
		{
			lod = 0;
		}
		else if (dist < lodDistances[1])
		{
			lod = 1;
		}
		else if (dist < lodDistances[2])
		{
			lod = 2;
		}
		else
		{
			lod = 3;
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

	memcpy(mappedMemory[currentFrame], instanceData.data(), sizeof(SpiralInstanceData) * config.instanceCount);
}

void SpiralScene::updateSpiralPositions(float deltaTime)
{
	animationTime += deltaTime * config.speed;
	float totalLength = config.instanceCount * config.spacing;

	for (uint32_t i = 0; i < config.instanceCount; i++)
	{
		// depth
		float linearPos = (i * config.spacing) + animationTime;
		float distance = fmod(linearPos, totalLength);
		float currentZ = -distance;

		// base angle of the arm
		// (i % numArms) ensures that instance 0 goes to arm 0, instance 1 to arm 1...
		float baseArmAngle = (i % config.numArms) * (6.28318f / config.numArms);

		// angle twist based on depth
		float twistAngle = abs(currentZ) * config.twistSpeed;

		// final angle
		float finalAngle = baseArmAngle + twistAngle;

		// radius
		float currentRadius = config.minRadius + (abs(currentZ) * config.coneFactor);

		glm::vec3 pos = glm::vec3(
			currentRadius * cos(finalAngle),
			currentRadius * sin(finalAngle),
			currentZ
		);

		positions[i] = pos;
	}

}

void SpiralScene::addModelType(const std::string& modelPath)
{
	generateLODVersions(modelPath);
}
