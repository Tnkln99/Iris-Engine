#include "DeferredRenderer.hpp"
#include "../Initializers.hpp"
#include "../Debugger.hpp"
#include "../AssetsManager.hpp"
#include "../../utilities/Timer.hpp"

namespace iris::graphics{

    DeferredRenderer::DeferredRenderer(Device &device, Window &window) : Renderer(device, window) {
        m_pSwapchain = std::make_unique<Swapchain>(device, window.getExtent());
    }

    DeferredRenderer::~DeferredRenderer() {
        vkDeviceWaitIdle(m_rDevice.getDevice());
        vkDestroyPipelineLayout(m_rDevice.getDevice(), m_lightPipelineLayout, nullptr);
        vkDestroyRenderPass(m_rDevice.getDevice(), m_renderPass, nullptr);
    }

    void DeferredRenderer::postRender() {
        vkDeviceWaitIdle(m_rDevice.getDevice());
        freeCommandBuffers();
    }

    void DeferredRenderer::init() {
        createCommandBuffers();
        initRenderPass();
        initGPassTextures();
        initGPassFramebuffer();
    }

    void DeferredRenderer::loadRenderer() {
        initLightPassPipeline();
        initGBufferDescriptorSets();
        initMaterials();
    }

    VkCommandBuffer DeferredRenderer::beginFrame() {
        uint32_t imageIndex = m_pSwapchain->acquireNextImage(getCurrentFrame());
        auto cmd = m_commandBuffers[getCurrentFrame()];
        //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
        Debugger::vkCheck(vkResetCommandBuffer(cmd, 0),
                          "Failed to reset command buffer!");

        VkCommandBufferBeginInfo beginInfo = Initializers::createCommandBufferBeginInfo();
        Debugger::vkCheck(vkBeginCommandBuffer(cmd, &beginInfo),
                          "Failed to begin recording command buffer!");

        // renderpass begin
        VkRenderPassBeginInfo renderPassInfo = Initializers::renderPassBeginInfo(m_renderPass,
                                                                                 m_pSwapchain->getExtent(),
                                                                                 m_pSwapchain->getFrameBuffer(imageIndex));
        VkClearValue clearValue;
        clearValue.color = { { 0.5f, 0.5f, 0.5f, 1.0f } };

        //clear depth at 1
        VkClearValue depthClear;
        depthClear.depthStencil.depth = 1.f;
        //connect clear values
        renderPassInfo.clearValueCount = 2;

        VkClearValue clearValues[] = { clearValue, depthClear };

        renderPassInfo.pClearValues = &clearValues[0];

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_pSwapchain->getExtent().width);
        viewport.height = static_cast<float>(m_pSwapchain->getExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, m_pSwapchain->getExtent() };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        return cmd;
    }

    void
    DeferredRenderer::renderScene(std::vector<RenderObject> &renderObjects, GpuSceneData sceneData, Camera &camera) {
        VkCommandBuffer cmd = beginFrame();

        sceneData.m_projectionMatrix = camera.m_projectionMatrix;
        sceneData.m_viewMatrix = camera.m_viewMatrix;

        m_rDevice.copyToBuffer(&sceneData,
                               m_uboSceneBuffers[getCurrentFrame()],
                               sizeof(GpuSceneData));

        camera.update(getSwapchainExtent(),
                      utils::Timer::getDeltaTime());

        std::shared_ptr<Model> lastModel = nullptr;
        std::shared_ptr<Material> lastMaterial = nullptr;
        for(auto & renderObject : renderObjects){
            renderObject.updateInfo();
            auto material = renderObject.m_pMaterial;
            if(lastMaterial != material){
                material->getPipeline()->bind(cmd);

                vkCmdBindDescriptorSets(
                        cmd,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        material->getPipeLineLayout(),
                        0,
                        1,
                        &m_sceneDescriptorSets[getCurrentFrame()],
                        0,
                        nullptr
                );

                if(material->getTextureSet() != VK_NULL_HANDLE){
                    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            material->getPipeLineLayout(), 1, 1,
                                            &material->getTextureSet(), 0, nullptr);
                }

                lastMaterial = material;
            }


            vkCmdPushConstants(
                    cmd,
                    material->getPipeLineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(RenderObject::GpuObjectData),
                    &renderObject.m_gpuObjectData);


            if(lastModel != renderObject.m_pModel){
                renderObject.m_pModel->bind(cmd);
                lastModel = renderObject.m_pModel;
            }
            renderObject.m_pModel->draw(cmd);
        }

        m_lightPipeline->bind(cmd);

        m_screenQuad.bind(cmd);
        m_screenQuad.draw(cmd);
        endFrame(cmd);
    }

    void DeferredRenderer::endFrame(VkCommandBuffer cmd) {
        vkCmdEndRenderPass(cmd);
        Debugger::vkCheck(vkEndCommandBuffer(cmd), "Failed to record command buffer!");

        m_pSwapchain->submitCommandBuffers(&cmd, getCurrentFrame());
        m_frameCount++;
    }

    void DeferredRenderer::createCommandBuffers() {
        m_commandBuffers.resize(m_pSwapchain->m_cMaxImagesOnFlight);
        VkCommandBufferAllocateInfo allocInfo = Initializers::createCommandBufferAllocateInfo(m_rDevice.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()));
        Debugger::vkCheck(vkAllocateCommandBuffers(m_rDevice.getDevice(), &allocInfo, m_commandBuffers.data()),
                          "Failed to allocate command buffers!");
    }

    void DeferredRenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
                m_rDevice.getDevice(),
                m_rDevice.getCommandPool(),
                static_cast<uint32_t>(m_commandBuffers.size()),
                m_commandBuffers.data());
        m_commandBuffers.clear();
    }

    void DeferredRenderer::createImage(VkDevice device, VmaAllocator allocator,
                                       uint32_t width, uint32_t height,
                                       VkFormat format, VkImageUsageFlags usage,
                                       AllocatedImage &allocatedImage) {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &allocatedImage.m_image,
                           &allocatedImage.m_allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }
    }

    void
    DeferredRenderer::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                      VkImageView &imageView) {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void DeferredRenderer::transitionGBufferImageLayouts() {
        m_rDevice.immediateSubmit([this](VkCommandBuffer commandBuffer) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            // Transition layouts for albedo, normal, specular, and position images
            std::vector<VkImage> images = {m_albedoTexture.m_allocatedImage.m_image, m_normalTexture.m_allocatedImage.m_image, m_specularTexture.m_allocatedImage.m_image, m_positionTexture.m_allocatedImage.m_image};
            for (VkImage image : images) {
                barrier.image = image;
                vkCmdPipelineBarrier(
                        commandBuffer,
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        0, // No dependency flags
                        0, nullptr, // No memory barriers
                        0, nullptr, // No buffer barriers
                        1, &barrier // Image barrier
                );
            }

            // Transition for depth image
            barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            barrier.image = m_depthTexture.m_allocatedImage.m_image;
            vkCmdPipelineBarrier(
                    commandBuffer,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
            );
        });
    }

    void DeferredRenderer::initGPassTextures() {
        VkFormat albedoFormat = VK_FORMAT_R8G8B8A8_UNORM;     // Albedo might be stored with standard color format
        VkFormat normalFormat = VK_FORMAT_R16G16B16A16_SFLOAT; // Normals need high precision
        VkFormat specularFormat = VK_FORMAT_R8G8B8A8_UNORM;    // Specular or roughness/metallic might also use standard color format
        VkFormat positionFormat = VK_FORMAT_R16G16B16A16_SFLOAT; // High precision for position data
        VkFormat depthFormat = m_pSwapchain->findDepthFormat();


        createImage(m_rDevice.getDevice(), m_rDevice.getAllocator(), m_pSwapchain->getExtent().width, m_pSwapchain->getExtent().height, albedoFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_albedoTexture.m_allocatedImage);
        createImage(m_rDevice.getDevice(), m_rDevice.getAllocator(), m_pSwapchain->getExtent().width, m_pSwapchain->getExtent().height, normalFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_normalTexture.m_allocatedImage);
        createImage(m_rDevice.getDevice(), m_rDevice.getAllocator(), m_pSwapchain->getExtent().width, m_pSwapchain->getExtent().height, specularFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_specularTexture.m_allocatedImage);
        createImage(m_rDevice.getDevice(), m_rDevice.getAllocator(), m_pSwapchain->getExtent().width, m_pSwapchain->getExtent().height, positionFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_positionTexture.m_allocatedImage);
        createImage(m_rDevice.getDevice(), m_rDevice.getAllocator(), m_pSwapchain->getExtent().width, m_pSwapchain->getExtent().height, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_depthTexture.m_allocatedImage);

        createImageView(m_rDevice.getDevice(), m_albedoTexture.m_allocatedImage.m_image, albedoFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_albedoTexture.m_imageView);
        createImageView(m_rDevice.getDevice(), m_normalTexture.m_allocatedImage.m_image, normalFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_normalTexture.m_imageView);
        createImageView(m_rDevice.getDevice(), m_specularTexture.m_allocatedImage.m_image, specularFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_specularTexture.m_imageView);
        createImageView(m_rDevice.getDevice(), m_positionTexture.m_allocatedImage.m_image, positionFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_positionTexture.m_imageView);
        createImageView(m_rDevice.getDevice(), m_depthTexture.m_allocatedImage.m_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, m_depthTexture.m_imageView);

        transitionGBufferImageLayouts();
    }

    void DeferredRenderer::initGPassFramebuffer() {
        // Assuming the textures are already created and have valid image views.
        std::vector<VkImageView> attachments;
        attachments.resize(6);
        attachments[0] = m_albedoTexture.m_imageView;  // Albedo attachment
        attachments[1] = m_specularTexture.m_imageView;  // Normal attachment
        attachments[2] = m_normalTexture.m_imageView; // Specular attachment
        attachments[3] = m_positionTexture.m_imageView; // Position attachment
        attachments[4] = m_depthTexture.m_imageView;   // Depth attachment
        attachments[5] = VK_NULL_HANDLE;

        // Create the framebuffer for the G-buffer pass
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_pSwapchain->getExtent().width;
        framebufferInfo.height = m_pSwapchain->getExtent().height;
        framebufferInfo.layers = 1;


        if (vkCreateFramebuffer(m_rDevice.getDevice(), &framebufferInfo, nullptr, &m_gBufferFramebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create G-buffer framebuffer");
        }

        m_pSwapchain->createFramebufferWithAttachments(m_renderPass, attachments.size());
    }

    void DeferredRenderer::initRenderPass() {
        // Create attachment descriptions for G-buffer components and final output
        std::vector<VkAttachmentDescription> attachments = {
                Initializers::createAttachmentDescription(VK_FORMAT_R16G16B16A16_SFLOAT,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), // Albedo
                Initializers::createAttachmentDescription(VK_FORMAT_R16G16B16A16_SFLOAT,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), // Normal
                Initializers::createAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),     // Specular
                Initializers::createAttachmentDescription(VK_FORMAT_R16G16B16A16_SFLOAT,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), // Position
                Initializers::createAttachmentDescription(m_pSwapchain->findDepthFormat(),  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),// Depth
                Initializers::createAttachmentDescription(m_pSwapchain->getSwapchainImageFormat(),  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) // Final output
        };

        // Reference to G-buffer attachments in subpass 1
        VkAttachmentReference albedoRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference normalRef = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference specularRef = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference positionRef = {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthRef = {4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        std::vector<VkAttachmentReference> gBufferAttachmentRefs = { albedoRef, normalRef, specularRef, positionRef };

        // Setup for subpass 1 (G-buffer generation)
        VkSubpassDescription gBufferSubpass = {};
        gBufferSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        gBufferSubpass.colorAttachmentCount = static_cast<uint32_t>(gBufferAttachmentRefs.size());
        gBufferSubpass.pColorAttachments = gBufferAttachmentRefs.data();
        gBufferSubpass.pDepthStencilAttachment = &depthRef;

        // Reference to final color attachment in subpass 2
        VkAttachmentReference finalColorRef = {5, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        // Setup for subpass 2 (lighting calculation using G-buffer)
        VkSubpassDescription lightingSubpass = {};
        lightingSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        lightingSubpass.colorAttachmentCount = 1;
        lightingSubpass.pColorAttachments = &finalColorRef;

        // Setup subpass dependencies for correct ordering and layout transitions
        std::vector<VkSubpassDependency> dependencies = {
                {
                        .srcSubpass = VK_SUBPASS_EXTERNAL,
                        .dstSubpass = 0,
                        .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .srcAccessMask = 0,
                        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
                },
                {
                        .srcSubpass = 0,
                        .dstSubpass = 1,
                        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
                },
                {
                        .srcSubpass = 1,
                        .dstSubpass = VK_SUBPASS_EXTERNAL,
                        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
                }
        };

        std::vector<VkSubpassDescription> subpasses { gBufferSubpass, lightingSubpass };

        // Create the render pass with both subpasses
        VkRenderPassCreateInfo renderPassInfo = Initializers::createRenderPassInfo(attachments, subpasses, dependencies);
        if (vkCreateRenderPass(m_rDevice.getDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create the render pass.");
        }
    }


    void DeferredRenderer::initGBufferDescriptorSets() {
        // initialize the global descriptor sets
        m_sceneDescriptorSets.resize(getMaximumFramesInFlight());
        m_uboSceneBuffers.resize(getMaximumFramesInFlight());

        m_pGlobalPool = DescriptorPool::Builder(m_rDevice)
                .setMaxSets(100)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
                .build();

        for(auto & uboBuffer : m_uboSceneBuffers)
        {
            uboBuffer = m_rDevice.createBuffer(sizeof(GpuSceneData),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                               VMA_MEMORY_USAGE_CPU_TO_GPU);
        }

        m_pGlobalSetLayout = DescriptorSetLayout::Builder(m_rDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        for (int i = 0; i < m_sceneDescriptorSets.size(); i++)
        {
            VkDescriptorBufferInfo sceneInfo;
            sceneInfo.buffer = m_uboSceneBuffers[i].m_buffer;
            sceneInfo.offset = 0;
            sceneInfo.range = sizeof(GpuSceneData);

            DescriptorWriter(*m_pGlobalSetLayout, *m_pGlobalPool)
                    .writeBuffer(0, &sceneInfo)
                    .build(m_sceneDescriptorSets[i]);
        }

        m_pTexturedSetLayout = DescriptorSetLayout::Builder(m_rDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();
    }

    void DeferredRenderer::initMaterials() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(RenderObject::GpuObjectData);

        VkPipelineLayoutCreateInfo texturedPipelineLayoutCreateInfo = Initializers::createPipelineLayoutInfo();
        const std::vector texturedDescriptorSetLayouts{m_pGlobalSetLayout->getDescriptorSetLayout(), m_pTexturedSetLayout->getDescriptorSetLayout()};

        texturedPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(texturedDescriptorSetLayouts.size());
        texturedPipelineLayoutCreateInfo.pSetLayouts = texturedDescriptorSetLayouts.data();
        texturedPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        texturedPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        VkPipelineLayout texturedPipelineLayout{};

        Debugger::vkCheck(vkCreatePipelineLayout(m_rDevice.getDevice(),
                                                 &texturedPipelineLayoutCreateInfo,
                                                 nullptr, &texturedPipelineLayout),"Failed to create pipeline layout");

        assert(texturedPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        graphics::PipelineConfigInfo texturedPipelineConfigInfo{};

        graphics::Pipeline::defaultPipelineConfig(texturedPipelineConfigInfo);

        texturedPipelineConfigInfo.m_renderPass = m_renderPass;
        texturedPipelineConfigInfo.m_pipelineLayout = texturedPipelineLayout;

        auto texturedPipeline = std::make_shared<Pipeline>(
                m_rDevice,
                "../shaders/DeferredGeometry.vert.spv",
                "../shaders/DeferredGeometry.frag.spv",
                texturedPipelineConfigInfo);

        AssetsManager::loadMaterial(m_rDevice, "DefaultMeshTextured", texturedPipeline, texturedPipelineLayout);
        auto mat = AssetsManager::getMaterial("DefaultMeshTextured");
        mat->setTexture(AssetsManager::getTexture("StarAmbient"),
                        AssetsManager::getTexture("StarDiffuse"),
                        AssetsManager::getTexture("StarSpecular"),
                        *m_pGlobalPool, *m_pTexturedSetLayout);
    }

    void DeferredRenderer::initLightPassPipeline() {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Initializers::createPipelineLayoutInfo();
        Debugger::vkCheck(vkCreatePipelineLayout(m_rDevice.getDevice(), &pipelineLayoutCreateInfo, nullptr, &m_lightPipelineLayout),
                          "Failed to create pipeline layout");
        assert(m_lightPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        graphics::PipelineConfigInfo pipelineConfig{};
        graphics::Pipeline::defaultPipelineConfig(pipelineConfig);
        pipelineConfig.m_bindingDescriptions = ScreenQuad::getBindingDescriptions();
        pipelineConfig.m_attributeDescriptions = ScreenQuad::getAttributeDescriptions();

        pipelineConfig.m_renderPass = m_renderPass;
        pipelineConfig.m_pipelineLayout = m_lightPipelineLayout;

        m_lightPipeline = std::make_shared<Pipeline>(
                m_rDevice,
                "../shaders/DeferredLight.vert.spv",
                "../shaders/DeferredLight.frag.spv",
                pipelineConfig);
    }
}