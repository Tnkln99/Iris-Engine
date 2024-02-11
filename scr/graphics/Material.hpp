#ifndef IRIS_MATERIAL_HPP
#define IRIS_MATERIAL_HPP

#include "Pipeline.hpp"
#include "Descriptors.hpp"

#include <vulkan/vulkan.h>
#include <string>
#include <memory>

namespace iris::graphics{
    class Material {
    public:
        struct MaterialInstance{
            MaterialInstance(const std::shared_ptr<Material>& material);
            MaterialInstance(const std::shared_ptr<Material>& material,
                             const std::shared_ptr<Texture>& ambientTexture,
                             const std::shared_ptr<Texture>& diffuseTexture,
                             const std::shared_ptr<Texture>& specularTexture,
                             const DescriptorPool& pool, const DescriptorSetLayout& layout);
            ~MaterialInstance() = default;
            std::shared_ptr<Material> m_pMaterial;
            VkDescriptorSet m_textureSet{};
        };

        Material(Device& device, const std::string& name,
                 std::shared_ptr<Pipeline> pipeline,
                 VkPipelineLayout layout);
        ~Material();

        [[nodiscard]] const std::string& getName() const { return m_name; }
        [[nodiscard]] const std::shared_ptr<Pipeline>& getPipeline() const { return m_pipeline; }
        [[nodiscard]] const VkPipelineLayout& getPipeLineLayout() const { return m_pipeLineLayout; }

        Device& m_rDevice;
    private:
        std::string m_name{};
        VkSampler m_sampler{};

        std::shared_ptr<Pipeline> m_pipeline;
        VkPipelineLayout m_pipeLineLayout{};
    };
}



#endif //IRIS_MATERIAL_HPP
