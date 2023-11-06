#include "Material.hpp"
#include "Initializers.hpp"

#include <utility>

namespace iris::graphics{

    Material::Material(Device& device,
                       const std::string& name,
                       std::shared_ptr<Pipeline> pipeline,
                       VkPipelineLayout layout, VkDescriptorSet texture) : m_rDevice{device} {
        m_Name = name;
        m_TextureSet = texture;
        m_Pipeline = std::move(pipeline);
        m_PipeLineLayout = layout;
    }

    void Material::setTexture(std::shared_ptr<Texture> texture, const DescriptorPool &pool, const DescriptorSetLayout &layout) {
        pool.allocateDescriptor(layout.getDescriptorSetLayout(),
                                         m_TextureSet);
        m_Texture = texture;
        VkSamplerCreateInfo samplerInfo = Initializers::createSamplerInfo(VK_FILTER_LINEAR);
        VkSampler sampler;
        vkCreateSampler(m_rDevice.getDevice(), &samplerInfo, nullptr, &sampler);

        VkDescriptorImageInfo imageBufferInfo;
        imageBufferInfo.sampler = sampler;
        imageBufferInfo.imageView = texture->imageView;
        imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet texture1 = Initializers::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_TextureSet, &imageBufferInfo, 0);
        vkUpdateDescriptorSets(m_rDevice.getDevice(), 1, &texture1, 0, nullptr);

    }
}