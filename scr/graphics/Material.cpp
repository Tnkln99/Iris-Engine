#include "Material.hpp"
#include "Initializers.hpp"

#include <utility>
#include <vector>

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

    void Material::setTexture(const std::shared_ptr<Texture>& ambientTexture,
                              const std::shared_ptr<Texture>& diffuseTexture,
                              const std::shared_ptr<Texture>& specularTexture,
                              const DescriptorPool &pool, const DescriptorSetLayout &layout) {

        pool.allocateDescriptor(layout.getDescriptorSetLayout(),
                                m_textureSet);

        m_pAmbientTexture = ambientTexture;
        m_pDiffuseTexture = diffuseTexture;
        m_pSpecularTexture = specularTexture;

        VkDescriptorImageInfo ambientImageBufferInfo;
        ambientImageBufferInfo.sampler = m_sampler;
        ambientImageBufferInfo.imageView = ambientTexture->m_imageView;
        ambientImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo diffuseImageBufferInfo;
        diffuseImageBufferInfo.sampler = m_sampler;
        diffuseImageBufferInfo.imageView = diffuseTexture->m_imageView;
        diffuseImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo specularImageBufferInfo;
        specularImageBufferInfo.sampler = m_sampler;
        specularImageBufferInfo.imageView = specularTexture->m_imageView;
        specularImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet texture1 = Initializers::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_textureSet, &ambientImageBufferInfo, 0);
        VkWriteDescriptorSet texture2 = Initializers::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_textureSet, &diffuseImageBufferInfo, 1);
        VkWriteDescriptorSet texture3 = Initializers::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_textureSet, &specularImageBufferInfo, 2);

        std::vector<VkWriteDescriptorSet> descriptorWrites = {texture1, texture2, texture3};

        vkUpdateDescriptorSets(m_rDevice.getDevice(), 3, descriptorWrites.data(), 0, nullptr);
    }

}