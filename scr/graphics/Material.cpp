#include "Material.hpp"
#include "Initializers.hpp"

#include <utility>

namespace iris::graphics{

    Material::Material(Device& device,
                       const std::string& name,
                       std::shared_ptr<Pipeline> pipeline,
                       VkPipelineLayout layout, VkDescriptorSet texture) : m_rDevice{device} {
        m_name = name;
        m_textureSet = texture;
        m_pipeline = std::move(pipeline);
        m_pipeLineLayout = layout;

        VkSamplerCreateInfo samplerInfo = Initializers::createSamplerInfo(VK_FILTER_LINEAR);
        vkCreateSampler(m_rDevice.getDevice(), &samplerInfo, nullptr, &m_sampler);
    }


    Material::~Material() {
        vkDestroySampler(m_rDevice.getDevice(), m_sampler, nullptr);
    }

    void Material::setTexture(std::shared_ptr<Texture> diffuseTexture, const DescriptorPool &pool, const DescriptorSetLayout &layout) {
        pool.allocateDescriptor(layout.getDescriptorSetLayout(),
                                m_textureSet);
        m_pTexture = diffuseTexture;

        VkDescriptorImageInfo imageBufferInfo;
        imageBufferInfo.sampler = m_sampler;
        imageBufferInfo.imageView = diffuseTexture->m_imageView;
        imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet texture1 = Initializers::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_textureSet, &imageBufferInfo, 0);
        vkUpdateDescriptorSets(m_rDevice.getDevice(), 1, &texture1, 0, nullptr);
    }

}