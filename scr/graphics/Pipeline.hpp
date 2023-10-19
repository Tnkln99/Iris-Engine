#ifndef IRIS_PIPELINE_HPP
#define IRIS_PIPELINE_HPP

#include "Device.hpp"

namespace iris::graphics{
    struct PipelineConfigInfo{
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
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
        VkPipeline m_GraphicsPipeline;
        VkShaderModule m_VertShaderModule;
        VkShaderModule m_FragShaderModule;

        static std::vector<char> readFile(const std::string& filePath);

        void createGraphicPipeline(const std::string& vertFilePath,
                                   const std::string& fragFilePath,
                                   const PipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
    };
}


#endif //IRIS_PIPELINE_HPP
