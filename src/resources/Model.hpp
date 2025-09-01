#pragma once
#include <vulkan/vulkan.hpp>
#include "../core/Device.hpp"
#include "Buffer.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../../external/tinygltf/tiny_gltf.h"

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

class Mesh {
public:
	Mesh(const Device& device, const tinygltf::Model& model, const tinygltf::Primitive& primitive);
	void draw(vk::CommandBuffer commandBuffer) const;

private:
	void loadVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<Vertex>& vertices);
	void loadIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<uint32_t>& indices);
	void createVertexBuffer(const std::vector<Vertex>& vertices);
	void createIndexBuffer(const std::vector<uint32_t>& indices);

	const Device& dev;
	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;
	uint32_t indexCount;
	uint32_t vertexCount;
};

class Model {
public:
	Model(const Device& device, const std::string& modelPath);
	~Model() = default;

	void draw(vk::CommandBuffer commandBuffer) const;

private:
	void loadModel(const std::string& modelPath);
	void processNode(const tinygltf::Node& node);
	void processMesh(const tinygltf::Mesh& mesh);

	const Device& dev;
	tinygltf::Model model;
	std::vector<std::unique_ptr<Mesh>> meshes;
};