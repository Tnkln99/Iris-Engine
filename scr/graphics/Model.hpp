#ifndef IRIS_MODEL_HPP
#define IRIS_MODEL_HPP

#include <memory>
#include "Device.hpp"

namespace iris::utils{
    // from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
        seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };
}


namespace iris::graphics{
    class Model
    {
    public:
        struct Vertex {
            glm::vec3 m_position{};
            glm::vec3 m_color{};
            glm::vec3 m_normal{};
            glm::vec2 m_uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            Vertex() = default;
            Vertex(glm::vec3 pos, glm::vec3 col, glm::vec3 norm, glm::vec2 tex)
                    : m_position(pos), m_color(col), m_normal(norm), m_uv(tex) {}

            bool operator==(const Vertex& other) const
            {
                return m_position == other.m_position && m_color == other.m_color && m_normal == other.m_normal &&
                       m_uv == other.m_uv;
            }
        };
        struct Builder {
            std::vector<Vertex> m_vertices{};
            std::vector<uint32_t> m_indices{};

            void loadModel(const std::string& filePath);
        };

        Model(Device& device, const Builder& builder);
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filePath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        std::vector<Vertex> m_vertices{};
        std::vector<uint32_t> m_indices{};
        Device& m_rDevice;
    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);


        AllocatedBuffer m_vertexBuffer{};
        uint32_t m_vertexCount{};

        bool m_hasIndexBuffer = false;
        AllocatedBuffer m_indexBuffer{};
        uint32_t m_indexCount{};
    };
}



#endif //IRIS_MODEL_HPP
