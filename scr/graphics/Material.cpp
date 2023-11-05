#include "Material.hpp"

#include <utility>

namespace iris::graphics{

    Material::Material(const std::string& name,
                       std::shared_ptr<Pipeline> pipeline,
                       VkPipelineLayout layout, VkDescriptorSet texture) {
        m_Name = name;
        m_TextureSet = texture;
        m_Pipeline = std::move(pipeline);
        m_PipeLineLayout = layout;
    }
}