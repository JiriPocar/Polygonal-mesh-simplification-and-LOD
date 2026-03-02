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

struct MeshData {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

class Mesh {
public:
	Mesh(Device& device, const tinygltf::Model& model, const tinygltf::Primitive& primitive);
	Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	void draw(vk::CommandBuffer commandBuffer) const;

	vk::Buffer getVertexBuffer() const { return vertexBuffer->getBuffer(); }
	vk::Buffer getIndexBuffer() const { return indexBuffer->getBuffer(); }
	uint32_t getVertexCount() const { return vertexCount; }
	uint32_t getIndexCount() const { return indexCount; }

	std::vector<Vertex> extractVertices() const;
	std::vector<uint32_t> extractIndices() const;

	void getBounds(glm::vec3& minBound, glm::vec3& maxBound) const;

private:
	void loadVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<Vertex>& vertices);
	void loadIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<uint32_t>& indices);
	void createVertexBuffer(const std::vector<Vertex>& vertices);
	void createIndexBuffer(const std::vector<uint32_t>& indices);

	void calculateMeshBounds(const std::vector<Vertex>& vertices);
	glm::vec3 minBound;
	glm::vec3 maxBound;

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

	void draw(vk::CommandBuffer commandBuffer) const;
	float getScaleIndex() const;

private:
	void loadModel(const std::string& modelPath, CommandManager& cmd);
	void processMesh(const tinygltf::Mesh& mesh);
	void createFromData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	uint32_t indexCount;
	uint32_t vertexCount;
	Device& dev;
	tinygltf::Model model;
	std::vector<std::unique_ptr<Mesh>> meshes;
	std::unique_ptr<Texture> texture;
};