/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Descriptors.hpp
 * @brief Descriptor set management for Vulkan application.
 *
 * This file contains the implementation of the Descriptor class,
 * which is responsible for creating and managing Vulkan descriptor
 * sets and descriptor set layouts for both graphics and compute pipelines.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include "UniformBuffer.hpp"
#include "../core/Device.hpp"

class Texture;

class Descriptor {
public:
	Descriptor(Device& device, UniformBuffer& uniformBuffer, uint32_t maxFrames = 2);
	~Descriptor() = default;

	/**
	* @brief Updates the texture descriptor for a specific frame.
	* 
	* @param frame The index of the frame
	* @param texture The texture to bind to the descriptor
	*/
	void updateTexture(uint32_t frame, Texture& texture);

	/**
	* @brief Creates compute shader descriptors for the given scene.
	* 
	* @param scene Spiral scene containing the buffers to bind to the compute shader
	* @param maxFrames The maximum number of frames to create descriptors for
	*/
	void createComputeDescriptors(class SpiralScene& scene, uint32_t maxFrames = 2);

	/**
	* @brief Updates the compute shader descriptors, when max instances are changed and buffers are recreated.
	* 
	* @param scene Spiral scene containing the buffers to bind to the compute shader
	* @param maxFrames The maximum number of frames to create descriptors for
	*/
	void updateDescriptorsCompute(class SpiralScene& scene, uint32_t maxFrames = 2);

	// getters
	vk::DescriptorSet get(uint32_t frame) const { return *descriptorSets[frame]; }
	vk::DescriptorSet getCompute(uint32_t frame) const { return *computeDescriptorSets[frame]; };
	vk::DescriptorSetLayout getLayout() const { return *descriptorSetLayout; }
	vk::DescriptorSetLayout getComputeLayout() const { return *computeDescriptorSetLayout; }

private:
	// creates the descriptor set layout for the graphics pipeline
	void createDescriptorSetLayout();

	/**
	* @brief Creates the descriptor pool for allocating descriptor sets.
	* 
	* @param maxFrames The maximum number of frames to create descriptors for
	*/
	void createDescriptorPool(uint32_t maxFrames);

	/**
	* @brief Creates descriptor sets for the graphics pipeline and updates them with the uniform buffer.
	* 
	* @param uniformBuffer The uniform buffer to bind to the descriptor sets
	* @param maxFrames The maximum number of frames to create descriptors for
	*/
	void createDescriptorSets(UniformBuffer& uniformBuffer, uint32_t maxFrames);

	Device& dev;
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
	vk::UniqueDescriptorSetLayout computeDescriptorSetLayout;
	vk::UniqueDescriptorPool descriptorPool;
	std::vector<vk::UniqueDescriptorSet> computeDescriptorSets; // per frame
	std::vector<vk::UniqueDescriptorSet> descriptorSets; // per frame
};

/* End of the Descriptors.hpp file */