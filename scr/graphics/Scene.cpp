#include "Scene.hpp"
#include "Initializers.hpp"
#include "Debugger.hpp"
#include "../utilities/Timer.hpp"
#include "AssetsManager.hpp"

#include <cassert>

namespace iris::graphics{

    Scene::Scene(Device& device, Renderer &renderer) : m_rDevice{device}, m_rRenderer{renderer} {
        loadScene();
        utils::Timer::init();
    }


    Scene::~Scene() {
        vkDestroyPipelineLayout(m_rDevice.getDevice(), m_DefaultPipelineLayout, nullptr);

        for (auto & uboBuffer : m_UboCameraBuffers)
        {
            m_rDevice.destroyBuffer(uboBuffer);
        }
    }

    void Scene::draw() {
        VkCommandBuffer cmd = m_rRenderer.beginFrame();

        m_DefaultPipeline->bind(cmd);
        vkCmdBindDescriptorSets(
                cmd,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_DefaultPipelineLayout,
                0,
                1,
                &m_GlobalDescriptorSets[m_rRenderer.getCurrentFrame()],
                0,
                nullptr
        );

        m_rDevice.copyToBuffer(&m_Camera.m_GpuCameraData,
                               m_UboCameraBuffers[m_rRenderer.getCurrentFrame()],
                               sizeof(GpuCameraData));

        vkCmdPushConstants(
                cmd,
                m_DefaultPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(GpuObjectData),
                &m_BoatObject.m_GpuObjectData);

        m_Camera.update(m_rRenderer.getSwapchainExtent(), utils::Timer::getDeltaTime());
        m_BoatObject.update();


        m_BoatObject.m_Model->bind(cmd);
        m_BoatObject.m_Model->draw(cmd);

        m_rRenderer.endFrame(cmd);
    }

    void Scene::loadScene() {
        // initialize the global descriptor sets
        m_GlobalDescriptorSets.resize(m_rRenderer.getMaximumFramesInFlight());
        m_UboCameraBuffers.resize(m_rRenderer.getMaximumFramesInFlight());

        m_GlobalPool = DescriptorPool::Builder(m_rDevice)
                .setMaxSets(m_rRenderer.getMaximumFramesInFlight())
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_rRenderer.getMaximumFramesInFlight())
                .build();

        for(auto & uboBuffer : m_UboCameraBuffers)
        {
            uboBuffer = m_rDevice.createBuffer(sizeof(GpuCameraData),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VMA_MEMORY_USAGE_CPU_TO_GPU);
        }

        const auto globalSetLayout =
                DescriptorSetLayout::Builder(m_rDevice)
                        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .build();

        for (int i = 0; i < m_GlobalDescriptorSets.size(); i++)
        {
            VkDescriptorBufferInfo cameraInfo;
            cameraInfo.buffer = m_UboCameraBuffers[i].buffer;
            cameraInfo.offset = 0;
            cameraInfo.range = sizeof(GpuCameraData);

            DescriptorWriter(*globalSetLayout, *m_GlobalPool)
                    .writeBuffer(0, &cameraInfo)
                    .build(m_GlobalDescriptorSets[i]);
        }

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Initializers::createPipelineLayoutInfo();
        const std::vector descriptorSetLayout{ globalSetLayout->getDescriptorSetLayout() };

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(GpuObjectData);

        pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayout.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        Debugger::vkCheck(vkCreatePipelineLayout(m_rDevice.getDevice(), &pipelineLayoutCreateInfo, nullptr, &m_DefaultPipelineLayout),
                          "Failed to create pipeline layout");

        assert(m_DefaultPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        graphics::PipelineConfigInfo pipelineConfig{};

        graphics::Pipeline::defaultPipelineConfig(pipelineConfig);

        pipelineConfig.renderPass = m_rRenderer.getRenderPass();
        pipelineConfig.pipelineLayout = m_DefaultPipelineLayout;
        m_DefaultPipeline = std::make_unique<graphics::Pipeline>(
                m_rDevice,
                "../shaders/Default.vert.spv",
                "../shaders/Default.frag.spv",
                pipelineConfig);


        AssetsManager::loadModel(m_rDevice, "Boat", "../assets/models/lowPolyBoat/lowPolyBoat.obj");

        m_BoatObject.m_Model = AssetsManager::getModel("Boat");
        m_BoatObject.m_Transform.translation = {0.0f, 0.0f, 0.0f};
        m_BoatObject.m_Transform.scale = {0.1f, 0.1f, 0.1f};
    }
}