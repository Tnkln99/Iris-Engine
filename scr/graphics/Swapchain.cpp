#include <limits>
#include "Swapchain.hpp"
#include "Debugger.hpp"
#include "Initializers.hpp"

namespace iris::graphics{

    //////////////////////////////
    //Constructor and destructor//
    //////////////////////////////

    Swapchain::Swapchain(Device &device, VkExtent2D extent) : m_rDevice{device}, m_windowExtent{extent} {
        createSwapchain();
        createImageViews();
        createDepthResources();
        createSyncObjects();
    }

    Swapchain::~Swapchain() {
        for (auto imageView : m_swapchainImageViews) {
            vkDestroyImageView(m_rDevice.getDevice(), imageView, nullptr);
        }
        m_swapchainImageViews.clear();

        vkDestroySwapchainKHR(m_rDevice.getDevice(), m_swapchain, nullptr);


        for (int i = 0; i < m_depthImages.size(); i++) {
            vkDestroyImageView(m_rDevice.getDevice(), m_depthImageViews[i], nullptr);
            vkDestroyImage(m_rDevice.getDevice(), m_depthImages[i], nullptr);
            vkFreeMemory(m_rDevice.getDevice(), m_depthImageMemories[i], nullptr);
        }

        for (auto framebuffer : m_swapchainFramebuffers) {
            vkDestroyFramebuffer(m_rDevice.getDevice(), framebuffer, nullptr);
        }

        // cleanup synchronization objects
        for (size_t i = 0; i < m_cMaxImagesOnFlight; i++) {
            vkDestroySemaphore(m_rDevice.getDevice(), m_renderSemaphores[i], nullptr);
            vkDestroySemaphore(m_rDevice.getDevice(), m_presentSemaphores[i], nullptr);
            vkDestroyFence(m_rDevice.getDevice(), m_inFlightFences[i], nullptr);
        }
    }

    ///////////////////////
    //  Private methods //
    /////////////////////

    void Swapchain::createSwapchain() {
        SwapChainSupportDetails swapChainSupport = m_rDevice.getSwapChainSupport();
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.m_formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.m_presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.m_capabilities);

        uint32_t imageCount = swapChainSupport.m_capabilities.minImageCount + 1;
        if (swapChainSupport.m_capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.m_capabilities.maxImageCount) {
            imageCount = swapChainSupport.m_capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_rDevice.getSurface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = m_rDevice.getQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = {indices.m_graphicsFamily.value(), indices.m_presentFamily.value()};

        if (indices.m_graphicsFamily != indices.m_presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }

        createInfo.preTransform = swapChainSupport.m_capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        Debugger::vkCheck(vkCreateSwapchainKHR(m_rDevice.getDevice(), &createInfo, nullptr, &m_swapchain),
                          "Failed to create swapchain!");

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(m_rDevice.getDevice(), m_swapchain, &imageCount, nullptr);
        m_swapchainImages.resize(imageCount);

        vkGetSwapchainImagesKHR(m_rDevice.getDevice(), m_swapchain, &imageCount, m_swapchainImages.data());

        m_swapchainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
    }

    void Swapchain::createImageViews() {
        m_swapchainImageViews.resize(getImagesCount());
        for (size_t i = 0; i < m_swapchainImages.size(); i++) {
            VkImageViewCreateInfo viewInfo = Initializers::createImageViewInfo(
                    m_swapchainImageFormat,
                    m_swapchainImages[i],
                    VK_IMAGE_ASPECT_COLOR_BIT);
            Debugger::vkCheck(vkCreateImageView(m_rDevice.getDevice(), &viewInfo, nullptr, &m_swapchainImageViews[i]),
                              "Failed to create image view!");
        }
    }

    void Swapchain::createDepthResources() {
        VkFormat depthFormat = findDepthFormat();
        m_swapChainDepthFormat = depthFormat;
        VkExtent2D swapChainExtent = m_swapChainExtent;

        m_depthImages.resize(getImagesCount());
        m_depthImageMemories.resize(getImagesCount());
        m_depthImageViews.resize(getImagesCount());

        for (int i = 0; i < getImagesCount(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_rDevice.createImageWithInfo(
                    imageInfo,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_depthImages[i],
                    m_depthImageMemories[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            Debugger::vkCheck(vkCreateImageView(m_rDevice.getDevice(), &viewInfo, nullptr, &m_depthImageViews[i]),
                              "Failed to create image view!");
        }
    }

    void Swapchain::createSyncObjects() {
        m_presentSemaphores.resize(m_cMaxImagesOnFlight);
        m_renderSemaphores.resize(m_cMaxImagesOnFlight);
        m_inFlightFences.resize(m_cMaxImagesOnFlight);
        m_imagesInFlight.resize(getImagesCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_cMaxImagesOnFlight; i++) {
            if (vkCreateSemaphore(m_rDevice.getDevice(), &semaphoreInfo, nullptr, &m_presentSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_rDevice.getDevice(), &semaphoreInfo, nullptr, &m_renderSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_rDevice.getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                std::cout << "Present mode: Mailbox" << std::endl;
                return availablePresentMode;
            }
        }

        // for (const auto &availablePresentMode : availablePresentModes) {
        //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        //     std::cout << "Present mode: Immediate" << std::endl;
        //     return availablePresentMode;
        //   }
        // }

        std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = m_windowExtent;
            actualExtent.width = std::max(
                    capabilities.minImageExtent.width,
                    std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(
                    capabilities.minImageExtent.height,
                    std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    ///////////////////////
    //  Public methods  //
    /////////////////////


    uint32_t Swapchain::acquireNextImage(int currentFrame) {
        vkWaitForFences(
                m_rDevice.getDevice(),
                1,
                &m_inFlightFences[currentFrame],
                VK_TRUE,
                std::numeric_limits<uint64_t>::max());

        Debugger::vkCheck(vkAcquireNextImageKHR(
                                  m_rDevice.getDevice(),
                                  m_swapchain,
                                  std::numeric_limits<uint64_t>::max(),
                                  m_presentSemaphores[currentFrame],  // must be a not signaled semaphore
                VK_NULL_HANDLE,
                                  &m_swapchainImageIndex)
                , "Failed to acquire next image!");
        return m_swapchainImageIndex;
    }

    void Swapchain::createFramebuffers(VkRenderPass renderPass) {
        m_swapchainFramebuffers.resize(getImagesCount());

        for (size_t i = 0; i < getImagesCount(); i++) {
            std::vector<VkImageView> attachments = {m_swapchainImageViews[i], m_depthImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo = Initializers::createFramebufferInfo(renderPass,
                                                                                          m_windowExtent,
                                                                                          attachments);
            Debugger::vkCheck(vkCreateFramebuffer(m_rDevice.getDevice(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]),
                              "Failed to create framebuffer!");
        }
    }

    void Swapchain::createFramebufferWithAttachments(VkRenderPass renderPass, int attachmentsCount) {
        m_swapchainFramebuffers.resize(getImagesCount());


        for (size_t i = 0; i < getImagesCount(); i++) {
            std::vector<VkImageView> attachments;
            for(int i = 0; i < attachmentsCount; i++){
                if(i != attachmentsCount - 1){
                    attachments.push_back(VK_NULL_HANDLE);
                }
                else{
                    attachments.push_back(m_swapchainImageViews[i]);
                }
            }
            VkFramebufferCreateInfo framebufferInfo = Initializers::createFramebufferInfo(renderPass,
                                                                                          m_windowExtent,
                                                                                          attachments);
            Debugger::vkCheck(vkCreateFramebuffer(m_rDevice.getDevice(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]),
                              "Failed to create framebuffer!");
        }


    }

    void Swapchain::submitCommandBuffers(const VkCommandBuffer *buffers, int currentFrameIndex) {
        if (m_imagesInFlight[currentFrameIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_rDevice.getDevice(), 1, &m_imagesInFlight[currentFrameIndex], VK_TRUE, UINT64_MAX);
        }
        m_imagesInFlight[currentFrameIndex] = m_inFlightFences[currentFrameIndex];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_presentSemaphores[currentFrameIndex]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = {m_renderSemaphores[currentFrameIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_rDevice.getDevice(), 1, &m_inFlightFences[currentFrameIndex]);
        if (vkQueueSubmit(m_rDevice.getGraphicsQueue(), 1, &submitInfo,
                          m_inFlightFences[currentFrameIndex]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_swapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &m_swapchainImageIndex;

        Debugger::vkCheck(vkQueuePresentKHR(m_rDevice.getPresentQueue(), &presentInfo),
                          "Failed to present image!");
    }
}
