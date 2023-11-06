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
        Material(Device& device, const std::string& name,
                 std::shared_ptr<Pipeline> pipeline,
                 VkPipelineLayout layout, VkDescriptorSet texture = VK_NULL_HANDLE);
        ~Material();

        void setTexture(std::shared_ptr<Texture> texture, const DescriptorPool& pool, const DescriptorSetLayout& layout);

        [[nodiscard]] const std::string& getName() const { return m_name; }
        [[nodiscard]] const VkDescriptorSet& getTextureSet() const { return m_textureSet; }
        [[nodiscard]] const std::shared_ptr<Pipeline>& getPipeline() const { return m_pipeline; }
        [[nodiscard]] const VkPipelineLayout& getPipeLineLayout() const { return m_pipeLineLayout; }
    private:
        Device& m_rDevice;

        std::string m_name{};

        VkSampler m_sampler{};
        std::shared_ptr<Texture> m_pTexture;
        VkDescriptorSet m_textureSet{VK_NULL_HANDLE}; //texture defaulted to null

        std::shared_ptr<Pipeline> m_pipeline;
        VkPipelineLayout m_pipeLineLayout{};
    };
}



#endif //IRIS_MATERIAL_HPP
