/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file CommandManager.hpp
 * @brief Command buffer management for Vulkan application.
 *
 * This file contains the implementation of the CommandManager class,
 * which is responsible for creating and managing Vulkan command buffers,
 * as well as providing utility functions for common command buffer operations.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class Device;

class CommandManager {
public:
	CommandManager(Device& device);
	~CommandManager() = default;

	/**
	 * @brief Begins recording a single-use command buffer.
	 * 
	 * @return vk::CommandBuffer command buffer ready for recording commands.
	*/
	vk::CommandBuffer beginSingleTimeCommands();

	/**
	 * @brief Ends recording of a single-use command buffer and submits it.
	 *
	 * @param commandBuffer The command buffer to end and submit.
	 *
	 * Ends the recording of the provided command buffer, submits, waits.
	*/
	void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

	/**
	 * @brief Creates the specified number of command buffers.
     *	
	 * @param count Number of command buffers to create.
	 * 
	 * Allocates the 'count' number of command buffers and stores
	 * them in the 'commandBuffers' member variable.
	*/
	void createCommandBuffers(uint32_t count);

	/**
	 * @brief Returns a command buffer at the specified index.
	 *
	 * @param index Command buffer index to return.
	 * @return vk::CommandBuffer Command buffer specified by the 'index'.
	 *
	 * Returns the command buffer at the specified index from the
	 * 'commandBuffers' vector.
	*/
	vk::CommandBuffer getCommandBuffer(uint32_t index) const;

	/**
	 * @brief Transition an image layout to the specified new layout.
	 *
	 * @param image Vulkan image to transition.
	 * @param format Format of the image, used to determine aspect mask.
	 * @param oldLayout Current layout of the image.
	 * @param newLayout Desired layout to transition the image to.
	 * 
	 * Inserts a barrier, which ensures that GPU will reorganize data from one state to another,
	 * and that the appropriate pipeline stages are synchronized.
	*/
	void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	
	/**
	 * @brief Copy data from a buffer to an image.
	 *
	 * @param buffer Vulkan buffer containing the data to copy.
	 * @param image Vulkan image to copy data into.
	 * @param width Width of the image region to copy.
	 * @param height Height of the image region to copy.
	 * 
	 * Records a command to copy data from the specified buffer to the given image.
	*/
	void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

private:
	Device& dev;
	vk::UniqueCommandPool commandPool;
	std::vector<vk::UniqueCommandBuffer> commandBuffers;
};

/* End of the CommandManager.hpp file */