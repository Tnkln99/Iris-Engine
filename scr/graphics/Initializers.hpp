#ifndef IRIS_INITIALIZERS_HPP
#define IRIS_INITIALIZERS_HPP

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace iris::graphics{
    class Initializers {
    public:
        [[nodiscard]] static VkApplicationInfo createAppInfo(const std::string& appName, const std::string& engineName);
        [[nodiscard]] static VkInstanceCreateInfo createInstanceInfo(VkApplicationInfo& appInfo,
                                                                     const std::vector<const char*>& extensions,
                                                                     const std::vector<const char*>& validations,
                                                                     bool enableValidationLayers);
        [[nodiscard]] static VkDeviceQueueCreateInfo createDeiceQueue(uint32_t queueFamilyIndex, uint32_t queueCount, float* queuePriorities);
        [[nodiscard]] static VkDeviceCreateInfo createDeviceInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
                                                                 const std::vector<const char*>& deviceExtensions,
                                                                 const std::vector<const char*>& validations,
                                                                 VkPhysicalDeviceFeatures& deviceFeatures,
                                                                 bool enableValidationLayers);
        [[nodiscard]] static VkCommandPoolCreateInfo createCommandPoolInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);
        [[nodiscard]] static VkCommandBufferAllocateInfo createCommandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount);
        [[nodiscard]] static VkCommandBufferBeginInfo createCommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
        [[nodiscard]] static VkRenderPassCreateInfo createRenderPassInfo(const std::vector<VkAttachmentDescription>& attachments,
                                                                         const std::vector<VkSubpassDescription>& subpasses,
                                                                         const std::vector<VkSubpassDependency>& dependencies);
        [[nodiscard]] static VkImageViewCreateInfo createImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) ;
        [[nodiscard]] static VkImageCreateInfo  createImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
        [[nodiscard]] static VkSamplerCreateInfo createSamplerInfo(VkFilter filters, VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
        [[nodiscard]] static VkFramebufferCreateInfo createFramebufferInfo(VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments);
        [[nodiscard]] static VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass,
                                                                       VkExtent2D windowExtent,
                                                                       VkFramebuffer framebuffer) ;
        [[nodiscard]] static VkPipelineLayoutCreateInfo createPipelineLayoutInfo();
        [[nodiscard]] static VkWriteDescriptorSet writeDescriptorImage(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);
    };
}



#endif //IRIS_INITIALIZERS_HPP
