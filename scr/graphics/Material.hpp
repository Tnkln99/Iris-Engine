#ifndef IRIS_MATERIAL_HPP
#define IRIS_MATERIAL_HPP

#include <vulkan/vulkan.h>

namespace iris::graphics{
    class Material {
    public:
        VkDescriptorSet m_TextureSet{VK_NULL_HANDLE}; //texture defaulted to null

        VkPipeline m_Pipeline{};
        VkPipelineLayout m_PipeLineLayout{};
    };
}



#endif //IRIS_MATERIAL_HPP
