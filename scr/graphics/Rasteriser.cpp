#include "Rasteriser.hpp"
#include "Initializers.hpp"
#include "Debugger.hpp"

#include <memory>

namespace iris::graphics{

    Rasteriser::Rasteriser(Device &device, Window& window)
    : m_rDevice{device}, m_rWindow{window}
    {
        auto extent = m_rWindow.getExtent();
        m_pSwapchain = std::make_unique<Swapchain>(device, extent);

        createCommandBuffers();
        createRenderPass();
        m_pSwapchain->createFramebuffers(m_renderPass);
    }


    Rasteriser::~Rasteriser() {
        vkDeviceWaitIdle(m_rDevice.getDevice());
        freeCommandBuffers();
        vkDestroyRenderPass(m_rDevice.getDevice(), m_renderPass, nullptr);
    }

    void Rasteriser::createCommandBuffers() {
        m_commandBuffers.resize(m_pSwapchain->m_cMaxImagesOnFlight);
        VkCommandBufferAllocateInfo allocInfo = Initializers::createCommandBufferAllocateInfo(m_rDevice.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()));
        Debugger::vkCheck(vkAllocateCommandBuffers(m_rDevice.getDevice(), &allocInfo, m_commandBuffers.data()),
                          "Failed to allocate command buffers!");
    }

    void Rasteriser::createRenderPass() {
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

    VkCommandBuffer Rasteriser::beginFrame() {
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

    void Rasteriser::endFrame(VkCommandBuffer cmd) {
        vkCmdEndRenderPass(cmd);
        Debugger::vkCheck(vkEndCommandBuffer(cmd), "Failed to record command buffer!");

        m_pSwapchain->submitCommandBuffers(&cmd, getCurrentFrame());
        m_frameCount++;
    }

    void Rasteriser::freeCommandBuffers() {
        vkFreeCommandBuffers(
                m_rDevice.getDevice(),
                m_rDevice.getCommandPool(),
                static_cast<uint32_t>(m_commandBuffers.size()),
                m_commandBuffers.data());
        m_commandBuffers.clear();
    }

    void Rasteriser::postRender() {
        vkDeviceWaitIdle(m_rDevice.getDevice());
    }
}