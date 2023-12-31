#include "Model.hpp"

#include <tiny_obj_loader.h>
#include <glm/gtx/hash.hpp>

#include <unordered_map>



namespace std
{
    template<>
    struct hash<iris::graphics::Model::Vertex> {
        size_t operator()(iris::graphics::Model::Vertex const& vertex) const {
            size_t seed = 0;
            iris::utils::hashCombine(seed, vertex.m_position, vertex.m_color, vertex.m_normal, vertex.m_uv);
            return seed;
        }
    };
}

namespace iris::graphics{
    void Model::Builder::loadModel(const std::string& filePath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        m_vertices.clear();
        m_indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.m_position = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.m_color = {
                            attrib.colors[3 * index.vertex_index + 0],
                            attrib.colors[3 * index.vertex_index + 1],
                            attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.m_normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.m_uv = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                    m_vertices.push_back(vertex);
                }
                m_indices.push_back(uniqueVertices[vertex]);
            }
        }

        std::cout << "Model loaded successfully " << filePath << std::endl;
    }

    Model::Model(Device& device, const Builder& builder) : m_rDevice{device }
    {
        createVertexBuffers(builder.m_vertices);
        createIndexBuffers(builder.m_indices);
    }

    Model::~Model(){
        m_rDevice.destroyBuffer(m_vertexBuffer);
        if (m_hasIndexBuffer) {
            m_rDevice.destroyBuffer(m_indexBuffer);
        }
    }

    std::unique_ptr<Model> Model::createModelFromFile(Device& device, const std::string& filePath)
    {
        Builder builder{};
        builder.loadModel(filePath);
        return std::make_unique<Model>(device, builder);
    }

    void Model::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {m_vertexBuffer.m_buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void Model::draw(VkCommandBuffer commandBuffer)
    {
        if (m_hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
        }
    }

    void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
    {
        m_vertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_vertexCount >= 3 && "Vertex count mu st be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        AllocatedBuffer stagingBuffer = m_rDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                               VMA_MEMORY_USAGE_CPU_ONLY);

        m_rDevice.copyToBuffer((void*)vertices.data(), stagingBuffer, bufferSize);

        m_vertexBuffer = m_rDevice.createBuffer(bufferSize,
                                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                VMA_MEMORY_USAGE_GPU_ONLY);

        m_rDevice.immediateSubmit([=](VkCommandBuffer cmd) {
            VkBufferCopy copy;
            copy.dstOffset = 0;
            copy.srcOffset = 0;
            copy.size = bufferSize;
            vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m_vertexBuffer.m_buffer, 1, &copy);
        });

        m_rDevice.destroyBuffer(stagingBuffer);
    }

    void Model::createIndexBuffers(const std::vector<uint32_t>& indices)
    {
        m_indexCount = static_cast<uint32_t>(indices.size());
        m_hasIndexBuffer = m_indexCount > 0;

        if (!m_hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        AllocatedBuffer stagingBuffer = m_rDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                               VMA_MEMORY_USAGE_CPU_ONLY);

        m_rDevice.copyToBuffer((void*)indices.data(), stagingBuffer, bufferSize);

        m_indexBuffer = m_rDevice.createBuffer(bufferSize,
                                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               VMA_MEMORY_USAGE_GPU_ONLY);

        m_rDevice.immediateSubmit([=](VkCommandBuffer cmd) {
            VkBufferCopy copy;
            copy.dstOffset = 0;
            copy.srcOffset = 0;
            copy.size = bufferSize;
            vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m_indexBuffer.m_buffer, 1, &copy);
        });

        m_rDevice.destroyBuffer(stagingBuffer);
    }

    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_color) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_normal) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, m_uv) });

        return attributeDescriptions;
    }
}