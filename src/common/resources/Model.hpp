/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Model.hpp
 * @brief Model loading and management.
 *
 * This file implements the Model class, which is responsible for loading 3D models from files using tinygltf,
 * creating vertex and index buffers for the meshes, and providing functionality to draw the model using Vulkan command buffers.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include "../core/Device.hpp"
#include "Buffer.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../../external/tinygltf/tiny_gltf.h"
#include "../common/resources/Textures.hpp"

class CommandManager;

#define LEFT 0
#define RIGHT 1

// represents a vertex
struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	static vk::VertexInputBindingDescription getBindingDesc();
	static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDesc();

	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}
};

// represents a single mesh in the model
struct MeshData {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

class Mesh {
public:
	Mesh(Device& device, const tinygltf::Model& model, const tinygltf::Primitive& primitive);
	Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	/**
	* @brief Records draw commands for this mesh into the provided command buffer.
	* 
	* @param commandBuffer The command buffer to record draw commands into.
	*/
	void draw(vk::CommandBuffer commandBuffer) const;

	// getters
	vk::Buffer getVertexBuffer() const { return vertexBuffer->getBuffer(); }
	vk::Buffer getIndexBuffer() const { return indexBuffer->getBuffer(); }
	uint32_t getVertexCount() const { return vertexCount; }
	uint32_t getIndexCount() const { return indexCount; }
	void getBounds(glm::vec3& minBound, glm::vec3& maxBound) const;

	std::vector<Vertex> extractVertices() const;
	std::vector<uint32_t> extractIndices() const;

private:

	/**
	* @brief Loads vertex data into vertices vector
	* 
	* @param model The tinygltf model containing the mesh data
	* @param primitive The tinygltf primitive representing the mesh to load
	* @param vertices Output vector to store the loaded vertices
	*/
	void loadVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<Vertex>& vertices);
	
	/**
	* @brief Loads index data into indices vector
	* 
	* @param model The tinygltf model containing the mesh data
	* @param primitive The tinygltf primitive representing the mesh to load
	* @param indices Output vector to store the loaded indices
	*/
	void loadIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<uint32_t>& indices);
	
	/**
	* @brief Calculates the axis-aligned bounding box of the mesh based on its vertices
	* 
	* @param vertices The vertices of the mesh to calculate bounds for
	*/
	void calculateMeshBounds(const std::vector<Vertex>& vertices);
	glm::vec3 minBound;
	glm::vec3 maxBound;

	// creates vertex and index buffers from the loaded vertex and index data
	void createVertexBuffer(const std::vector<Vertex>& vertices);
	void createIndexBuffer(const std::vector<uint32_t>& indices);

	Device& dev;
	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;
	uint32_t indexCount;
	uint32_t vertexCount;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

class Model {
public:
	Model(Device& device, CommandManager& cmd, const std::string& modelPath);

	// copy constructor
	Model(const Model& other);

	// data constructor
	Model(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	
	// simplified meshes constructor
	Model(Device& device, const std::vector<MeshData>& simplifiedMeshesData);

	~Model() = default;

	/**
	* @brief Records draw commands for all meshes in the model into the provided command buffer.
	* 
	* @param commandBuffer The command buffer to record draw commands into.
	*/
	void draw(vk::CommandBuffer commandBuffer) const;

	// calculates a uniform scale factor to fit the model to 10x10x10 box
	float getScaleIndex() const;

	// getters and setters
	vk::Buffer getVertexBuffer() const {
		if (meshes.empty()) throw std::runtime_error("Model has no meshes");
		return meshes[0]->getVertexBuffer();
	}

	vk::Buffer getIndexBuffer() const {
		if (meshes.empty()) throw std::runtime_error("Model has no meshes");
		return meshes[0]->getIndexBuffer();
	}

	const std::vector<std::unique_ptr<Mesh>>& getMeshes() const { return meshes; }
	Texture* getTexture() const { return texture.get(); }
	std::unique_ptr<Texture> releaseTexture() { return std::move(texture); }
	void setTexture(std::unique_ptr<Texture> newTexture) { texture = std::move(newTexture); }
	bool hasTexture() const { return texture != nullptr; }

	std::vector<Vertex> extractVertices() const;
	std::vector<uint32_t> extractIndices() const;

	uint32_t getVertexCount() const { return vertexCount; };
	uint32_t getIndexCount() const { return indexCount; };

private:
	/**
	* @brief Loads a GLTF model from the specified file path and processes its meshes.
	* 
	* @param modelPath The file path to the GLTF model to load
	* @param cmd The CommandManager for texture loading (transition and copying)
	*/
	void loadModel(const std::string& modelPath, CommandManager& cmd);

	/**
	* @brief Loads a specific output .obj model file, exported by the simplificator, and creates a model from it.
	* 
	* @param modelPath The file path of the .obj model to load
	*/
	void loadObjModel(const std::string& modelPath);

	/**
	* @brief Processes a tinygltf::Mesh, saving them to meshes vector
	* 
	* @param mesh The tinygltf::Mesh to process
	*/
	void processMesh(const tinygltf::Mesh& mesh);

	/**
	* @brief Recreates model with mesh data from provided vertices and indices.
	* 
	* @param vertices The vertex data to create the model from
	* @param indices The index data to create the model from
	*/
	void createFromData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	uint32_t indexCount;
	uint32_t vertexCount;
	Device& dev;
	tinygltf::Model model;
	std::vector<std::unique_ptr<Mesh>> meshes;
	std::unique_ptr<Texture> texture;
};

/* End of the Model.hpp file */