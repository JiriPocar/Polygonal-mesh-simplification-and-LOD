#include "SpiralScene.hpp"
#include <cmath>
#include <iostream>
#include <chrono>

SpiralScene::SpiralScene(Device& dev, uint32_t instanceCount, const std::string& modelPath)
	: dev(dev), instanceCount(instanceCount)
{
	generateSpiralPositions();
	generateLODVersions(modelPath);
	createInstanceBuffer();
	std::cout << "SpiralScene created" << std::endl;
}

void SpiralScene::generateSpiralPositions()
{
	positions.clear();
	positions.reserve(instanceCount);
	instanceData.resize(instanceCount);

	for (int i = 0; i < instanceCount; i++)
	{
		// spiral parameter
		float t = i * 0.8f;

		float radius = t * 0.8f;
		float angle = t * 0.5f;

		glm::vec3 pos = glm::vec3(
			radius * cos(angle),
			0.4f * t, // upward movement
			radius * sin(angle)
		);

		positions.push_back(pos);

		instanceData[i].pos = pos;
		instanceData[i].modelTypeIndex = 0; // so far only one model type
		instanceData[i].lodLevel = 0; // default LOD level
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
	vk::DeviceSize bufferSize = sizeof(SpiralInstanceData) * instanceCount;

	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = bufferSize;
	bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	instanceBuffer = dev.operator*().createBuffer(bufferInfo);

	vk::MemoryRequirements memReq = dev.operator*().getBufferMemoryRequirements(instanceBuffer);

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = dev.findMemoryType(memReq.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	instanceBufferMemory = dev.operator*().allocateMemory(allocInfo);
	dev.operator*().bindBufferMemory(instanceBuffer, instanceBufferMemory, 0);

	mappedMemory = dev.operator*().mapMemory(
		instanceBufferMemory,
		0,
		VK_WHOLE_SIZE,
		vk::MemoryMapFlags{}
	);
}

void SpiralScene::updateLODs(const glm::vec3& cameraPos)
{
	updateInstancesCPU(cameraPos);
}

void SpiralScene::updateInstancesCPU(const glm::vec3& cameraPos)
{
	for (int i = 0; i < instanceCount; i++)
	{
		float dist = glm::distance(cameraPos, positions[i]);

		if (dist < lodDistances[0])
		{
			instanceData[i].lodLevel = 0;
		}
		else if (dist < lodDistances[1])
		{
			instanceData[i].lodLevel = 1;
		}
		else if (dist < lodDistances[2])
		{
			instanceData[i].lodLevel = 2;
		}
		else
		{
			instanceData[i].lodLevel = 3;
		}
	}

	memcpy(mappedMemory, instanceData.data(), sizeof(SpiralInstanceData) * instanceCount);
}

void SpiralScene::updatePositions(float deltaTime)
{
	animationTime += deltaTime * animationSpeed;

	for (int i = 0; i < instanceCount; i++)
	{
		float t = (i * 0.5f) + animationTime;

		float radius = t * 0.8f;
		float angle = t * 0.5f;

		glm::vec3 pos = glm::vec3(
			radius * cos(angle),
			0.4f * t, // upward movement
			radius * sin(angle)
		);

		positions[i] = pos;
		instanceData[i].pos = pos;
	}

	memcpy(mappedMemory, instanceData.data(), sizeof(SpiralInstanceData) * instanceCount);
}

void SpiralScene::addModelType(const std::string& modelPath)
{
	generateLODVersions(modelPath);
}
