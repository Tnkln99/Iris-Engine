#ifndef IRIS_DEFERREDRENDERER_HPP
#define IRIS_DEFERREDRENDERER_HPP

#include "Renderer.hpp"
#include <vector>

struct QuadVertex{
    glm::vec3 pos;
    glm::vec2 texCoord;
};



namespace iris::graphics{
    struct ScreenQuad{
        ScreenQuad(Device& device) : m_rDevice{device}{
            createVertexBuffers(device);
            createIndexBuffers(device);
        }

        ~ScreenQuad(){
            m_rDevice.destroyBuffer(m_quadVertexBuffer);
            m_rDevice.destroyBuffer(m_quadIndexBuffer);
        }

        Device& m_rDevice;

        std::vector<QuadVertex> m_quadVertices = {
                {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
                {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}
        };
        uint32_t m_quadVertexCount = 4;

        std::vector<int> m_quadIndices= {
                0, 1, 2, // First Triangle
                2, 3, 0  // Second Triangle
        };
        uint32_t m_quadIndexCount = 6;

        AllocatedBuffer m_quadVertexBuffer{};
        AllocatedBuffer m_quadIndexBuffer{};

        void draw(VkCommandBuffer commandBuffer)
        {
            vkCmdDrawIndexed(commandBuffer, m_quadIndices.size(), 1, 0, 0, 0);
        }

        void bind(VkCommandBuffer commandBuffer)
        {
            VkBuffer buffers[] = {m_quadVertexBuffer.m_buffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_quadIndexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
        }

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(){
            std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
            bindingDescriptions[0].binding = 0;
            bindingDescriptions[0].stride = sizeof(QuadVertex);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescriptions;
        }

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(){
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

            attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(QuadVertex, pos) });
            attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(QuadVertex, texCoord) });

            return attributeDescriptions;
        }

        void createVertexBuffers(Device& device)
        {
            VkDeviceSize bufferSize = sizeof(m_quadVertices[0]) * m_quadVertexCount;

            AllocatedBuffer stagingBuffer = device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                   VMA_MEMORY_USAGE_CPU_ONLY);

            device.copyToBuffer((void*)m_quadVertices.data(), stagingBuffer, bufferSize);

            m_quadVertexBuffer = device.createBuffer(bufferSize,
                                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                    VMA_MEMORY_USAGE_GPU_ONLY);

            device.immediateSubmit([=](VkCommandBuffer cmd) {
                VkBufferCopy copy;
                copy.dstOffset = 0;
                copy.srcOffset = 0;
                copy.size = bufferSize;
                vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m_quadVertexBuffer.m_buffer, 1, &copy);
            });

            device.destroyBuffer(stagingBuffer);
        }

        void createIndexBuffers(Device& device)
        {

            VkDeviceSize bufferSize = sizeof(m_quadIndices[0]) * m_quadIndexCount;
            uint32_t indexSize = sizeof(m_quadIndices[0]);

            AllocatedBuffer stagingBuffer = device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                   VMA_MEMORY_USAGE_CPU_ONLY);

            device.copyToBuffer((void*)m_quadIndices.data(), stagingBuffer, bufferSize);

            m_quadIndexBuffer = device.createBuffer(bufferSize,
                                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                   VMA_MEMORY_USAGE_GPU_ONLY);

            device.immediateSubmit([=](VkCommandBuffer cmd) {
                VkBufferCopy copy;
                copy.dstOffset = 0;
                copy.srcOffset = 0;
                copy.size = bufferSize;
                vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m_quadIndexBuffer.m_buffer, 1, &copy);
            });

            device.destroyBuffer(stagingBuffer);
        }
    };

    class DeferredRenderer : public Renderer{
    public:
        DeferredRenderer(Device& device, Window& window);
        ~DeferredRenderer() override;

        void init() override;

        VkCommandBuffer beginFrame() override;
        void loadRenderer() override;
        void endFrame(VkCommandBuffer cmd) override;
        void postRender() override;

        void renderScene(std::vector<RenderObject> & renderObjects, GpuSceneData sceneData, Camera & camera) override;
    private:
        void createCommandBuffers() override;
        void freeCommandBuffers() override;

        // init render pass with 2 sub pass
        VkRenderPass m_renderPass;
        void initRenderPass();

        void createImage(VkDevice device, VmaAllocator allocator,
                         uint32_t width, uint32_t height,
                         VkFormat format, VkImageUsageFlags usage,
                         AllocatedImage& allocatedImage);
        void createImageView(VkDevice device, VkImage image, VkFormat format,
                             VkImageAspectFlags aspectFlags, VkImageView& imageView);

        void transitionGBufferImageLayouts();
        // init textures that framebuffer will use
        void initGPassTextures();
        Texture m_albedoTexture;
        Texture m_specularTexture;
        Texture m_normalTexture;
        Texture m_positionTexture;
        Texture m_depthTexture;
        // creating framebuffers to write the position normal and albedo
        // it will use the renderpass compatible with these framebuffers to init them
        void initGPassFramebuffer();
        VkFramebuffer m_gBufferFramebuffer;

        std::unique_ptr<DescriptorPool> m_pGlobalPool{};

        std::unique_ptr<DescriptorSetLayout> m_pGlobalSetLayout{};
        std::unique_ptr<DescriptorSetLayout> m_pTexturedSetLayout{};

        std::vector<VkDescriptorSet> m_sceneDescriptorSets{};
        std::vector<AllocatedBuffer> m_uboSceneBuffers;


        void initGBufferDescriptorSets();
        // different materials for the objects for now only textured material
        // materials has the pipeline and pipelinelayout information for the objects
        void initMaterials();

        ScreenQuad m_screenQuad{m_rDevice};

        // void initLightFramebuffer(); we dont need this because swapchain going to populate this framebuffers
        // we don't need material structure here because light pass is the same for every material
        // we will send the vertex and index buffer of the screenQuad to the light pass
        void initLightPassPipeline();

        std::shared_ptr<Pipeline> m_lightPipeline;
        VkPipelineLayout m_lightPipelineLayout;
    };
}


#endif //IRIS_DEFERREDRENDERER_HPP
