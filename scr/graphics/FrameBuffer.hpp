#ifndef IRIS_FRAMEBUFFER_HPP
#define IRIS_FRAMEBUFFER_HPP

#include "Device.hpp"
namespace iris::graphics{

    class FrameBuffer {
        /**
        * @brief Encapsulates a single frame buffer attachment
        */
        struct FramebufferAttachment
        {
            VkImage m_image;
            VkDeviceMemory m_memory;
            VkImageView m_view;
            VkFormat m_format;
            VkImageSubresourceRange m_subresourceRange;
            VkAttachmentDescription m_description;

            /**
            * @brief Returns true if the attachment has a depth component
            */
            [[nodiscard]] bool hasDepth() const
            {
                std::vector<VkFormat> formats =
                        {
                                VK_FORMAT_D16_UNORM,
                                VK_FORMAT_X8_D24_UNORM_PACK32,
                                VK_FORMAT_D32_SFLOAT,
                                VK_FORMAT_D16_UNORM_S8_UINT,
                                VK_FORMAT_D24_UNORM_S8_UINT,
                                VK_FORMAT_D32_SFLOAT_S8_UINT,
                        };
                return std::find(formats.begin(), formats.end(), m_format) != std::end(formats);
            }

            /**
            * @brief Returns true if the attachment has a stencil component
            */
            [[nodiscard]] bool hasStencil() const
            {
                std::vector<VkFormat> formats =
                        {
                                VK_FORMAT_S8_UINT,
                                VK_FORMAT_D16_UNORM_S8_UINT,
                                VK_FORMAT_D24_UNORM_S8_UINT,
                                VK_FORMAT_D32_SFLOAT_S8_UINT,
                        };
                return std::find(formats.begin(), formats.end(), m_format) != std::end(formats);
            }

            /**
            * @brief Returns true if the attachment is a depth and/or stencil attachment
            */
            [[nodiscard]] bool isDepthStencil() const
            {
                return(hasDepth() || hasStencil());
            }

        };

        /**
        * @brief Describes the attributes of an attachment to be created
        */
        struct AttachmentCreateInfo
        {
            uint32_t m_width, m_height;
            uint32_t m_layerCount;
            VkFormat m_format;
            VkImageUsageFlags m_usage;
            VkSampleCountFlagBits m_imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
        };
    public:
        uint32_t m_width{}, m_height{};
        VkFramebuffer m_framebuffer{};
        VkRenderPass m_renderPass{};
        VkSampler m_sampler{};

        std::vector<FramebufferAttachment> m_attachments;

        explicit FrameBuffer(Device & device);
        ~FrameBuffer();
        /**
		* Add a new attachment described by createinfo to the frame buffer's attachment list
		*
		* @param createinfo Structure that specifies the framebuffer to be constructed
		*
		* @return Index of the new attachment
		*/
        uint32_t addAttachment(AttachmentCreateInfo createinfo);
        /**
		* Creates a default sampler for sampling from any of the framebuffer attachments
		* Applications are free to create their own samplers for different use cases
		*
		* @param magFilter Magnification filter for lookups
		* @param minFilter Minification filter for lookups
		* @param adressMode Addressing mode for the U,V and W coordinates
		*
		* @return VkResult for the sampler creation
		*/
        VkResult createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode);
        /**
		* Creates a default render pass setup with one sub pass
		*
		* @return VK_SUCCESS if all resources have been created successfully
		*/
        VkResult createRenderPass();
    private:
        Device & m_rDevice;
    };
}

#endif //IRIS_FRAMEBUFFER_HPP
