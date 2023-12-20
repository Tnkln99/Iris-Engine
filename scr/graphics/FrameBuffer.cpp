#include "FrameBuffer.hpp"
#include "Initializers.hpp"
#include "Debugger.hpp"

namespace iris::graphics{

    FrameBuffer::FrameBuffer(Device &device) : m_rDevice{device}{

    }

    FrameBuffer::~FrameBuffer() {
        for (auto attachment : m_attachments)
        {
            vkDestroyImage(m_rDevice.getDevice(), attachment.m_image, nullptr);
            vkDestroyImageView(m_rDevice.getDevice(), attachment.m_view, nullptr);
            vkFreeMemory(m_rDevice.getDevice(), attachment.m_memory, nullptr);
        }
        vkDestroySampler(m_rDevice.getDevice(), m_sampler, nullptr);
        vkDestroyRenderPass(m_rDevice.getDevice(), m_renderPass, nullptr);
        vkDestroyFramebuffer(m_rDevice.getDevice(), m_framebuffer, nullptr);
    }

    uint32_t FrameBuffer::addAttachment(FrameBuffer::AttachmentCreateInfo createinfo) {
        FramebufferAttachment attachment{};

        attachment.m_format = createinfo.m_format;

        VkImageAspectFlags aspectMask;

        // Select aspect mask and layout depending on usage

        // Color attachment
        if (createinfo.m_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        // Depth (and/or stencil) attachment
        if (createinfo.m_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            if (attachment.hasDepth())
            {
                aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if (attachment.hasStencil())
            {
                aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }

        assert(aspectMask > 0);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = createinfo.m_width;
        imageInfo.extent.height = createinfo.m_height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = createinfo.m_layerCount;
        imageInfo.format = createinfo.m_format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = createinfo.m_usage;
        imageInfo.samples = createinfo.m_imageSampleCount;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        m_rDevice.createImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                attachment.m_image,
                attachment.m_memory);

        attachment.m_subresourceRange = {};
        attachment.m_subresourceRange.aspectMask = aspectMask;
        attachment.m_subresourceRange.levelCount = 1;
        attachment.m_subresourceRange.layerCount = createinfo.m_layerCount;

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = attachment.m_image;
        viewInfo.viewType = (createinfo.m_layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.format = createinfo.m_format;
        viewInfo.subresourceRange = attachment.m_subresourceRange;
        viewInfo.subresourceRange.aspectMask = (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;

        Debugger::vkCheck(vkCreateImageView(m_rDevice.getDevice(), &viewInfo, nullptr, &attachment.m_view),
                          "Failed to create image view!");

        // Fill attachment description
        attachment.m_description = {};
        attachment.m_description.samples = createinfo.m_imageSampleCount;
        attachment.m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.m_description.storeOp = (createinfo.m_usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.m_description.format = createinfo.m_format;
        attachment.m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // Final layout
        // If not, final layout depends on attachment type
        if (attachment.hasDepth() || attachment.hasStencil())
        {
            attachment.m_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }
        else
        {
            attachment.m_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }


        m_attachments.push_back(attachment);

        return static_cast<uint32_t>(m_attachments.size() - 1);
    }

    VkResult FrameBuffer::createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode) {
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.pNext = nullptr;

        info.magFilter = magFilter;
        info.minFilter = minFilter;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = adressMode;
        info.addressModeV = adressMode;
        info.addressModeW = adressMode;
        info.mipLodBias = 0.0f;
        info.maxAnisotropy = 1.0f;
        info.minLod = 0.0f;
        info.maxLod = 1.0f;
        info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        return vkCreateSampler(m_rDevice.getDevice(), &info, nullptr, &m_sampler);
    }

    VkResult FrameBuffer::createRenderPass() {
        std::vector<VkAttachmentDescription> attachmentDescriptions;
        for (auto& attachment : m_attachments)
        {
            attachmentDescriptions.push_back(attachment.m_description);
        };

        // Collect attachment references
        std::vector<VkAttachmentReference> colorReferences;
        VkAttachmentReference depthReference = {};
        bool hasDepth = false;
        bool hasColor = false;

        uint32_t attachmentIndex = 0;

        for (auto& attachment : m_attachments)
        {
            if (attachment.isDepthStencil())
            {
                // Only one depth attachment allowed
                assert(!hasDepth);
                depthReference.attachment = attachmentIndex;
                depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                hasDepth = true;
            }
            else
            {
                colorReferences.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
                hasColor = true;
            }
            attachmentIndex++;
        };

        // Default render pass setup uses only one subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        if (hasColor)
        {
            subpass.pColorAttachments = colorReferences.data();
            subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        }
        if (hasDepth)
        {
            subpass.pDepthStencilAttachment = &depthReference;
        }

        // Use subpass dependencies for attachment layout transitions
        std::array<VkSubpassDependency, 2> dependencies{};

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create render pass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies.data();
        Debugger::vkCheck(vkCreateRenderPass(m_rDevice.getDevice(), &renderPassInfo, nullptr, &m_renderPass),
                          "Could not create renderpass!");

        std::vector<VkImageView> attachmentViews;
        for (auto attachment : m_attachments)
        {
            attachmentViews.push_back(attachment.m_view);
        }

        // Find. max number of layers across attachments
        uint32_t maxLayers = 0;
        for (auto attachment : m_attachments)
        {
            if (attachment.m_subresourceRange.layerCount > maxLayers)
            {
                maxLayers = attachment.m_subresourceRange.layerCount;
            }
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.pAttachments = attachmentViews.data();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
        framebufferInfo.width = m_width;
        framebufferInfo.height = m_height;
        framebufferInfo.layers = maxLayers;
        Debugger::vkCheck(vkCreateFramebuffer(m_rDevice.getDevice(), &framebufferInfo, nullptr, &m_framebuffer),
                          "Could not create framebuffer");

        return VK_SUCCESS;
    }
}
