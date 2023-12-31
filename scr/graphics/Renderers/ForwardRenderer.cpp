#include "ForwardRenderer.hpp"
#include "../AssetsManager.hpp"

namespace iris::graphics{

    ForwardRenderer::ForwardRenderer(Device &device, Window& window) : Renderer(device, window) {
        m_pSwapchain = std::make_unique<Swapchain>(device, window.getExtent());
    }

    void ForwardRenderer::init() {
        createCommandBuffers();
        createRenderPass();
        m_pSwapchain->createFramebuffers(m_renderPass);
    }


    ForwardRenderer::~ForwardRenderer() {
        vkDeviceWaitIdle(m_rDevice.getDevice());
        vkDestroyRenderPass(m_rDevice.getDevice(), m_renderPass, nullptr);
        for (auto & uboBuffer : m_uboSceneBuffers)
        {
            m_rDevice.destroyBuffer(uboBuffer);
        }
    }

    void ForwardRenderer::postRender() {
        vkDeviceWaitIdle(m_rDevice.getDevice());
        freeCommandBuffers();
    }

    void ForwardRenderer::createRenderPass() {
        // the renderpass will use this color attachment.
        VkAttachmentDescription colorAttachment = {};
        //the attachment will have the format needed by the swapchain
        colorAttachment.format = m_pSwapchain->getSwapchainImageFormat();
        //1 sample, we won't be doing MSAA (Multisample Anti-Aliasing)
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        // we Clear when this attachment is loaded
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // we keep the attachment stored when the renderpass ends
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        //we don't care about stencil
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        //we don't know or care about the starting layout of the attachment
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        //after the renderpass ends, the image has to be on a layout ready for display
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        //attachment number will index into the pAttachments array in the parent renderpass itself
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment = {};
        // Depth attachment
        depthAttachment.flags = 0;
        depthAttachment.format = m_pSwapchain->findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        //we are going to create 1 subpass, which is the minimum you can do
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        //hook the depth attachment into the subpass
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        std::vector<VkSubpassDescription> subpasses = { subpass };

        //1 dependency, which is from "outside" into the subpass. And we can read or write color
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        //dependency from outside to the subpass, making this subpass dependent on the previous renderpasses
        VkSubpassDependency depthDependency = {};
        depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        depthDependency.dstSubpass = 0;
        depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.srcAccessMask = 0;
        depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        //array of 2 dependencies, one for color, two for depth
        std::vector<VkSubpassDependency> dependencies = { dependency, depthDependency };

        //array of 2 attachments, one for the color, and other for depth
        std::vector<VkAttachmentDescription> attachments = { colorAttachment,depthAttachment };

        VkRenderPassCreateInfo renderPassInfo = Initializers::createRenderPassInfo(attachments, subpasses, dependencies);

        Debugger::vkCheck(vkCreateRenderPass(m_rDevice.getDevice() , &renderPassInfo, nullptr, &m_renderPass)
                , "Failed to create render pass!");
    }

    VkCommandBuffer ForwardRenderer::beginFrame() {
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


    void ForwardRenderer::loadRenderer() {
        initDescriptorSets();
        initMaterials();
    }

    void ForwardRenderer::endFrame(VkCommandBuffer cmd) {
        vkCmdEndRenderPass(cmd);
        Debugger::vkCheck(vkEndCommandBuffer(cmd), "Failed to record command buffer!");

        m_pSwapchain->submitCommandBuffers(&cmd, getCurrentFrame());
        m_frameCount++;
    }

    void ForwardRenderer::renderScene(std::vector<RenderObject> & renderObjects, GpuSceneData sceneData,
                                      Camera & camera) {
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

        endFrame(cmd);
    }

    void ForwardRenderer::createCommandBuffers() {
        m_commandBuffers.resize(m_pSwapchain->m_cMaxImagesOnFlight);
        VkCommandBufferAllocateInfo allocInfo = Initializers::createCommandBufferAllocateInfo(m_rDevice.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()));
        Debugger::vkCheck(vkAllocateCommandBuffers(m_rDevice.getDevice(), &allocInfo, m_commandBuffers.data()),
                          "Failed to allocate command buffers!");
    }

    void ForwardRenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
                m_rDevice.getDevice(),
                m_rDevice.getCommandPool(),
                static_cast<uint32_t>(m_commandBuffers.size()),
                m_commandBuffers.data());
        m_commandBuffers.clear();
    }

    void ForwardRenderer::initDescriptorSets() {
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

    void ForwardRenderer::initMaterials() {
        // non textured pipeline
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Initializers::createPipelineLayoutInfo();
        const std::vector descriptorSetLayouts{m_pGlobalSetLayout->getDescriptorSetLayout()};

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(RenderObject::GpuObjectData);

        pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        VkPipelineLayout defaultPipelineLayout{};

        Debugger::vkCheck(vkCreatePipelineLayout(m_rDevice.getDevice(), &pipelineLayoutCreateInfo, nullptr, &defaultPipelineLayout),
                          "Failed to create pipeline layout");

        assert(defaultPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        graphics::PipelineConfigInfo pipelineConfig{};

        graphics::Pipeline::defaultPipelineConfig(pipelineConfig);

        pipelineConfig.m_renderPass = getRenderPass();
        pipelineConfig.m_pipelineLayout = defaultPipelineLayout;

        auto defaultPipeline = std::make_shared<Pipeline>(
                m_rDevice,
                "../shaders/Default.vert.spv",
                "../shaders/Default.frag.spv",
                pipelineConfig);

        AssetsManager::loadMaterial(m_rDevice, "DefaultMeshNonTextured", defaultPipeline, defaultPipelineLayout);

        // textured pipeline
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

        assert(defaultPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        graphics::PipelineConfigInfo texturedPipelineConfigInfo{};

        graphics::Pipeline::defaultPipelineConfig(texturedPipelineConfigInfo);

        texturedPipelineConfigInfo.m_renderPass = getRenderPass();
        texturedPipelineConfigInfo.m_pipelineLayout = texturedPipelineLayout;

        auto texturedPipeline = std::make_shared<Pipeline>(
                m_rDevice,
                "../shaders/Default.vert.spv",
                "../shaders/DefaultTextured.frag.spv",
                texturedPipelineConfigInfo);

        AssetsManager::loadMaterial(m_rDevice, "DefaultMeshTextured", texturedPipeline, texturedPipelineLayout);
        auto mat = AssetsManager::getMaterial("DefaultMeshTextured");
        mat->setTexture(AssetsManager::getTexture("StarAmbient"),
                        AssetsManager::getTexture("StarDiffuse"),
                        AssetsManager::getTexture("StarSpecular"),
                        *m_pGlobalPool, *m_pTexturedSetLayout);
    }

}