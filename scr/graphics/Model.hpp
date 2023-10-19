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
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex& other) const
            {
                return position == other.position && color == other.color && normal == other.normal &&
                       uv == other.uv;
            }
        };
        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filePath);
        };

        Model(Device& device, const Builder& builder);
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filePath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        Device& m_rDevice;

        AllocatedBuffer m_VertexBuffer{};
        uint32_t m_VertexCount{};

        bool m_bHasIndexBuffer = false;
        AllocatedBuffer m_IndexBuffer{};
        uint32_t m_IndexCount{};
    };
}



#endif //IRIS_MODEL_HPP
