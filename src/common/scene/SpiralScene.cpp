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
	vk::DeviceSize bufferSize = sizeof(SpiralInstanceData) * instanceData.size();

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
	for (int i = 0; i < config.instanceCount; i++)
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

	memcpy(mappedMemory, instanceData.data(), sizeof(SpiralInstanceData) * config.instanceCount);
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
		instanceData[i].pos = pos;
	}

	memcpy(mappedMemory, instanceData.data(), sizeof(SpiralInstanceData) * config.instanceCount);
}

void SpiralScene::addModelType(const std::string& modelPath)
{
	generateLODVersions(modelPath);
}
