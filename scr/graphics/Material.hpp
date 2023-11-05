#ifndef IRIS_MATERIAL_HPP
#define IRIS_MATERIAL_HPP

#include "Pipeline.hpp"

#include <vulkan/vulkan.h>
#include <string>
#include <memory>

namespace iris::graphics{
    class Material {
    public:
        Material(const std::string& name,
                 std::shared_ptr<Pipeline> pipeline,
                 VkPipelineLayout layout, VkDescriptorSet texture = VK_NULL_HANDLE);

        std::string m_Name{};


        VkDescriptorSet m_TextureSet{VK_NULL_HANDLE}; //texture defaulted to null
        std::shared_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout m_PipeLineLayout{};
    };
}



#endif //IRIS_MATERIAL_HPP
