#ifndef IRIS_PIPELINE_HPP
#define IRIS_PIPELINE_HPP

#include "Device.hpp"

namespace iris::graphics{
    struct PipelineConfigInfo{
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> m_bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions{};
        VkPipelineViewportStateCreateInfo m_viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo m_rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo m_multisampleInfo;
        VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo m_colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo m_depthStencilInfo;
        std::vector<VkDynamicState> m_dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo m_dynamicStateInfo;
        VkPipelineLayout m_pipelineLayout = nullptr;
        VkRenderPass m_renderPass = nullptr;
        uint32_t m_subpass = 0;
    };
    class Pipeline
    {
    public:
        Pipeline() = delete;
        Pipeline(Device& device,
                 const std::string& vertFilePath,
                 const std::string& fragFilePath,
                 const PipelineConfigInfo& configInfo);
        ~Pipeline();
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfig(PipelineConfigInfo& configInfo);
    private:
        Device& m_rDevice;
        VkPipeline m_graphicsPipeline;
        VkShaderModule m_vertShaderModule;
        VkShaderModule m_fragShaderModule;

        static std::vector<char> readFile(const std::string& filePath);

        void createGraphicPipeline(const std::string& vertFilePath,
                                   const std::string& fragFilePath,
                                   const PipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
    };
}


#endif //IRIS_PIPELINE_HPP
