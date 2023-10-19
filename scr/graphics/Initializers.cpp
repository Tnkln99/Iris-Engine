#include "Initializers.hpp"

namespace iris::graphics{

    VkApplicationInfo Initializers::createAppInfo(const std::string &appName, const std::string &engineName) {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = engineName.c_str();
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0; // Or the version you need
        return appInfo;
    }

    VkInstanceCreateInfo
    Initializers::createInstanceInfo(VkApplicationInfo &appInfo,
                                     const std::vector<const char *>& extensions,
                                     const std::vector<const char *>& validations,
                                     bool enableValidationLayers) {
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        if(enableValidationLayers){
            createInfo.enabledLayerCount = static_cast<uint32_t>(validations.size());
            createInfo.ppEnabledLayerNames = validations.data();
        }
        else{
            createInfo.enabledLayerCount = 0;
        }
        return createInfo;
    }

    VkDeviceQueueCreateInfo
    Initializers::createDeiceQueue(uint32_t queueFamilyIndex, uint32_t queueCount, float *queuePriorities) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex; // index of the queue family to create
        queueCreateInfo.queueCount = queueCount; // number of queues to create
        queueCreateInfo.pQueuePriorities = queuePriorities; // Vulkan needs to know how to handle multiple queues. We will leave this at the default value of 1.0f
        return queueCreateInfo;
    }

    VkDeviceCreateInfo Initializers::createDeviceInfo(const std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos,
                                                      const std::vector<const char *> &deviceExtensions,
                                                      const std::vector<const char*>& validations,
                                                      VkPhysicalDeviceFeatures &deviceFeatures,
                                                      bool enableValidationLayers) {
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data(); // the queue create infos
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // number of queue create infos
        createInfo.pEnabledFeatures = &deviceFeatures; // the device features
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()); // number of enabled extensions
        createInfo.ppEnabledExtensionNames = deviceExtensions.data(); // the enabled extensions
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validations.size()); // number of enabled layers
            createInfo.ppEnabledLayerNames = validations.data(); // the enabled layers
        } else {
            createInfo.enabledLayerCount = 0;
        }
        return createInfo;
    }

    VkCommandPoolCreateInfo Initializers::createCommandPoolInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex; // the queue family that command buffers from this command pool will use
        poolInfo.flags = flags; // we want to be able to reset command buffers
        return poolInfo;
    }

    VkCommandBufferAllocateInfo
    Initializers::createCommandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount) {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool; // the command pool to allocate the command buffers from
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // the type of command buffer we want to allocate
        allocInfo.commandBufferCount = commandBufferCount; // the number of command buffers to allocate
        return allocInfo;
    }

    VkCommandBufferBeginInfo Initializers::createCommandBufferBeginInfo(VkCommandBufferUsageFlags flags){
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;

        info.pInheritanceInfo = nullptr;
        info.flags = flags;
        return info;
    }

    VkRenderPassCreateInfo
    Initializers::createRenderPassInfo(const std::vector<VkAttachmentDescription>& attachments,
                                       const std::vector<VkSubpassDescription>& subpasses,
                                       const std::vector<VkSubpassDependency>& dependencies) {
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // number of attachments
        renderPassInfo.pAttachments = attachments.data(); // the attachments
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size()); // number of subpasses
        renderPassInfo.pSubpasses = subpasses.data(); // the subpasses
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size()); // number of dependencies
        renderPassInfo.pDependencies = dependencies.data(); // the dependencies
        return renderPassInfo;
    }

    VkImageViewCreateInfo
    Initializers::createImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image; // the image we are creating the view for
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // type of image
        viewInfo.format = format; // format of the image data
        viewInfo.subresourceRange.aspectMask = aspectFlags; // which aspect of the image we are viewing
        viewInfo.subresourceRange.baseMipLevel = 0; // start mipmap level to view from
        viewInfo.subresourceRange.levelCount = 1; // number of mipmap levels to view
        viewInfo.subresourceRange.baseArrayLayer = 0; // start array level to view from
        viewInfo.subresourceRange.layerCount = 1; // number of array levels to view
        return viewInfo;
    }

    VkFramebufferCreateInfo
    Initializers::createFramebufferInfo(VkRenderPass renderPass,
                                        VkExtent2D extent,
                                        const std::vector<VkImageView>& attachments) {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;
        return framebufferInfo;
    }

    VkRenderPassBeginInfo
    Initializers::renderPassBeginInfo(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer) {
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass; // render pass to begin
        renderPassInfo.framebuffer = framebuffer; // framebuffer to render into
        renderPassInfo.renderArea.offset = { 0, 0 }; // start point of render pass in pixels
        renderPassInfo.renderArea.extent = windowExtent; // size of region to run render pass on (starting at offset)
        VkClearValue clearValue = { 0.01f, 0.01f, 0.01f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;
        return renderPassInfo;
    }

    VkPipelineLayoutCreateInfo Initializers::createPipelineLayoutInfo() {
        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.pNext = nullptr;

        //empty defaults
        info.flags = 0;
        info.setLayoutCount = 0;
        info.pSetLayouts = nullptr;
        info.pushConstantRangeCount = 0;
        info.pPushConstantRanges = nullptr;
        return info;
    }
}