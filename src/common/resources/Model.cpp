#include "Model.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../external/tinygltf/tiny_gltf.h"

vk::VertexInputBindingDescription Vertex::getBindingDesc()
{
	vk::VertexInputBindingDescription bindingDesc(
		0,
		sizeof(Vertex),
		vk::VertexInputRate::eVertex
	);

	return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::getAttributeDesc()
{
	std::array<vk::VertexInputAttributeDescription, 3> attributeDescs = {};

	// position
	attributeDescs[0] = vk::VertexInputAttributeDescription(
		0,
		0,
		vk::Format::eR32G32B32Sfloat,
		offsetof(Vertex, pos)
	);

	// color
	attributeDescs[1] = vk::VertexInputAttributeDescription(
		1,
		0,
		vk::Format::eR32G32B32Sfloat,
		offsetof(Vertex, normal)
	);

	// texture coordinates
	attributeDescs[2] = vk::VertexInputAttributeDescription(
		2,
		0,
		vk::Format::eR32G32Sfloat,
		offsetof(Vertex, texCoord)
	);

	return attributeDescs;
}

Mesh::Mesh(const Device& device, const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	: dev(device), indexCount(0), vertexCount(0)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	loadVertices(model, primitive, vertices);
	loadIndices(model, primitive, indices);

	this->vertices = vertices;
	this->indices = indices;

	calculateMeshBounds(vertices);

	createVertexBuffer(vertices);
	createIndexBuffer(indices);

	std::cout << "Loaded mesh with " << vertexCount << " vertices and " << indexCount << " indices." << std::endl;
	if (!vertices.empty()) {
		glm::vec3 minPos = vertices[0].pos;
		glm::vec3 maxPos = vertices[0].pos;
		for (const auto& v : vertices) {
			minPos = glm::min(minPos, v.pos);
			maxPos = glm::max(maxPos, v.pos);
		}
		std::cout << "(" << minPos.x << "," << minPos.y << "," << minPos.z << ") to "
			<< "(" << maxPos.x << "," << maxPos.y << "," << maxPos.z << ")" << std::endl;
	}
}

Mesh::Mesh(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: dev(device),
	  indexCount(static_cast<uint32_t>(indices.size())),
	  vertexCount(static_cast<uint32_t>(vertices.size())),
	  vertices(vertices),
	  indices(indices)
{
	calculateMeshBounds(vertices);
	createVertexBuffer(vertices);
	createIndexBuffer(indices);
}

std::vector<Vertex> Mesh::extractVertices() const
{
	return vertices;
}

std::vector<uint32_t> Mesh::extractIndices() const
{
	return indices;
}

void Mesh::loadVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<Vertex>& vertices)
{
	if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
		throw std::runtime_error("Mesh missing POSITION attribute");
	}

	const auto& positionsAccessor = model.accessors[primitive.attributes.at("POSITION")];
	const auto& positionsView = model.bufferViews[positionsAccessor.bufferView];
	const float* positionsData = reinterpret_cast<const float*>(
		&model.buffers[positionsView.buffer].data[positionsView.byteOffset + positionsAccessor.byteOffset]
		);

	// normals (optional)
	bool hasNormals = primitive.attributes.find("NORMAL") != primitive.attributes.end();
	std::vector<float> normalsData;

	if (hasNormals) {
		const auto& normalsAccessor = model.accessors[primitive.attributes.at("NORMAL")];
		const auto& normalsView = model.bufferViews[normalsAccessor.bufferView];
		const float* normalsRawData = reinterpret_cast<const float*>(
			&model.buffers[normalsView.buffer].data[normalsView.byteOffset + normalsAccessor.byteOffset]
			);
		normalsData.assign(normalsRawData, normalsRawData + normalsAccessor.count * 3);
	}

	// texcoords (optional)
	bool hasTexCoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
	std::vector<float> texCoordsData;

	if (hasTexCoords) {
		const auto& texCoordsAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
		const auto& texCoordsView = model.bufferViews[texCoordsAccessor.bufferView];
		const float* texCoordsRawData = reinterpret_cast<const float*>(
			&model.buffers[texCoordsView.buffer].data[texCoordsView.byteOffset + texCoordsAccessor.byteOffset]
			);
		texCoordsData.assign(texCoordsRawData, texCoordsRawData + texCoordsAccessor.count * 2);
	}

	// Create vertices
	vertexCount = positionsAccessor.count;
	vertices.resize(vertexCount);

	for (size_t i = 0; i < vertexCount; i++)
	{
		vertices[i].pos = glm::vec3(
			positionsData[i * 3 + 0],
			positionsData[i * 3 + 1],
			positionsData[i * 3 + 2]
		);

		if (hasNormals)
		{
			vertices[i].normal = glm::vec3(
				normalsData[i * 3 + 0],
				normalsData[i * 3 + 1],
				normalsData[i * 3 + 2]
			);
		}
		else // default to up vector if no normals are provided
		{
			vertices[i].normal = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		
		if (hasTexCoords)
		{
			vertices[i].texCoord = glm::vec2(
				texCoordsData[i * 2 + 0],
				texCoordsData[i * 2 + 1]
			);
		}
		else // default to zero if no texcoords are provided
		{
			vertices[i].texCoord = glm::vec2(0.0f, 0.0f);
		}
	}
}

void Mesh::loadIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<uint32_t>& indices)
{
	// no indices -> create a default index buffer
	if (primitive.indices < 0)
	{
		indices.resize(vertexCount);

		for (uint32_t i = 0; i < vertexCount; i++)
		{
			indices[i] = i;
		}

		indexCount = vertexCount;
		return;
	}

	const auto& indicesAccessor = model.accessors[primitive.indices];
	const auto& indicesView = model.bufferViews[indicesAccessor.bufferView];

	indexCount = indicesAccessor.count;
	indices.resize(indexCount);

	if (indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
	{
		const uint16_t* indicesData = reinterpret_cast<const uint16_t*>(
			&model.buffers[indicesView.buffer].data[indicesView.byteOffset + indicesAccessor.byteOffset]);

		for (size_t i = 0; i < indexCount; i++)
		{
			indices[i] = indicesData[i];
		}
	}
	else if (indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
	{
		const uint32_t* indicesData = reinterpret_cast<const uint32_t*>(
			&model.buffers[indicesView.buffer].data[indicesView.byteOffset + indicesAccessor.byteOffset]);

		for (size_t i = 0; i < indexCount; i++)
		{
			indices[i] = indicesData[i];
		}
	}
	else
	{
		throw std::runtime_error("Unsupported index component type");
	}
}

void Mesh::createVertexBuffer(const std::vector<Vertex>& vertices)
{
	vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	vertexBuffer = std::make_unique<Buffer>(
		dev,
		bufferSize,
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	vertexBuffer->copyData(vertices.data(), bufferSize);
}

void Mesh::createIndexBuffer(const std::vector<uint32_t>& indices)
{
	vk::DeviceSize bufferSize = sizeof(uint32_t) * indices.size();

	indexBuffer = std::make_unique<Buffer>(
		dev,
		bufferSize,
		vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	indexBuffer->copyData(indices.data(), bufferSize);
}

void Mesh::draw(vk::CommandBuffer commandBuffer) const
{
	vk::Buffer vertexBuffers[] = { vertexBuffer->getBuffer() };
	vk::DeviceSize offsets[] = { 0 };

	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
	commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}

void Mesh::calculateMeshBounds(const std::vector<Vertex>& vertices)
{
	if (vertices.empty()) return;

	minBound = vertices[0].pos;
	maxBound = vertices[0].pos;

	for (const auto& v : vertices) {
		minBound = glm::min(minBound, v.pos);
		maxBound = glm::max(maxBound, v.pos);
	}
}

void Mesh::getBounds(glm::vec3& minBound, glm::vec3& maxBound) const
{
	minBound = this->minBound;
	maxBound = this->maxBound;
}

Model::Model(const Device& device, const std::string& modelPath)
	: dev(device)
{
	loadModel(modelPath);

	// set vertex and index count
	auto v = extractVertices();
	auto i = extractIndices();
	vertexCount = static_cast<uint32_t>(v.size());
	indexCount = static_cast<uint32_t>(i.size());
}

Model::Model(const Model& other) : dev(other.dev), indexCount(other.indexCount), vertexCount(other.vertexCount)
{
	auto vertices = other.extractVertices();
	auto indices = other.extractIndices();

	createFromData(vertices, indices);
}

Model::Model(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: dev(device), vertexCount(vertices.size()), indexCount(indices.size())
{
	createFromData(vertices, indices);
}

void Model::loadModel(const std::string& modelPath)
{

	std::ifstream testFile(modelPath);
	if (!testFile.is_open()) {
		throw std::runtime_error("File does not exist or cannot be opened: " + modelPath);
	}
	testFile.close();

	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, modelPath);

	if (!warn.empty()) {
		std::cout << "GLTF Warning: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << "GLTF Error: " << err << std::endl;
	}

	if (!ret) {
		throw std::runtime_error("Failed to load GLTF model: " + modelPath);
	}

	for (const auto& mesh : model.meshes) {
		processMesh(mesh);
	}

	std::cout << "Loaded GLTF model: " << modelPath << std::endl;
}

void Model::processMesh(const tinygltf::Mesh& mesh)
{
	for (const auto& primitive : mesh.primitives) {
		if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
		{
			std::cout << "Skipping non-triangular primitive\n" << std::endl;
			continue;
		}

		meshes.push_back(std::make_unique<Mesh>(dev, model, primitive));
	}
}

void Model::draw(vk::CommandBuffer commandBuffer) const
{
	for (const auto& mesh : meshes) {
		mesh->draw(commandBuffer);
	}
}

float Model::getScaleIndex() const
{
	if (meshes.empty()) return 1.0f;

	glm::vec3 min = glm::vec3(FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX);

	for (const auto& m : meshes)
	{
		glm::vec3 meshMin, meshMax;
		m->getBounds(meshMin, meshMax);

		min = glm::min(min, meshMin);
		max = glm::max(max, meshMax);
	}

	glm::vec3 size = max - min;
	float scaleCoordinate = glm::max(size.x, glm::max(size.y, size.z));

	return 10.0f / scaleCoordinate;
}

void Model::createFromData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	if (vertices.empty() || indices.empty())
	{
		throw std::runtime_error("Vertices or indices are empty.");
	}

	meshes.clear();
	meshes.push_back(std::make_unique<Mesh>(dev, vertices, indices));

	vertexCount = static_cast<uint32_t>(vertices.size());
	indexCount = static_cast<uint32_t>(indices.size());
}

std::vector<Vertex> Model::extractVertices() const
{
	std::vector<Vertex> allVertices;

	for (const auto& mesh : meshes)
	{
		auto meshVertices = mesh->extractVertices();
		allVertices.insert(allVertices.end(), meshVertices.begin(), meshVertices.end());
	}

	return allVertices;
}

std::vector<uint32_t> Model::extractIndices() const
{
	std::vector<uint32_t> allIndices;

	// ! due to multiple meshes, we need to offset the indices
	// the lantern X fox models incident
	uint32_t vertexOffset = 0;

	for (const auto& mesh : meshes)
	{
		auto meshIndices = mesh->extractIndices();

		for (auto& index : meshIndices)
		{
			index += vertexOffset;
		}

		allIndices.insert(allIndices.end(), meshIndices.begin(), meshIndices.end());

		vertexOffset += mesh->getVertexCount();
	}

	return allIndices;
}