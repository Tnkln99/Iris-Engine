#include "Scene.hpp"
#include "Initializers.hpp"
#include "Debugger.hpp"
#include "../utilities/Timer.hpp"
#include "AssetsManager.hpp"

#include <cassert>

namespace iris::graphics{

    Scene::Scene(Device& device, ForwardRenderer &renderer) : m_rDevice{device}, m_rRenderer{renderer} {
        m_rRenderer.init();
        utils::Timer::init();
    }


    Scene::~Scene() {
        for (auto & uboBuffer : m_uboSceneBuffers)
        {
            m_rDevice.destroyBuffer(uboBuffer);
        }
        m_renderObjects.clear();
    }

    void Scene::draw() {
        VkCommandBuffer cmd = m_rRenderer.beginFrame();

        m_sceneData.m_projectionMatrix = m_camera.m_projectionMatrix;
        m_sceneData.m_viewMatrix = m_camera.m_viewMatrix;
        m_sceneData.m_ambientLightColor = {1.f, 1.f, 1.f, .02f};
        m_sceneData.m_lightPosition = {1.0f, 1.0f, 1.0f};
        m_sceneData.m_lightColor = {0.0f, 1.0f, 1.0f, 1.0f};

        m_rDevice.copyToBuffer(&m_sceneData,
                               m_uboSceneBuffers[m_rRenderer.getCurrentFrame()],
                               sizeof(GpuSceneData));

        m_camera.update(m_rRenderer.getSwapchainExtent(), utils::Timer::getDeltaTime());

        std::shared_ptr<Model> lastModel = nullptr;
        std::shared_ptr<Material> lastMaterial = nullptr;
        for(auto & renderObject : m_renderObjects){
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
                        &m_sceneDescriptorSets[m_rRenderer.getCurrentFrame()],
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


        m_rRenderer.endFrame(cmd);
    }

    void Scene::loadScene() {
        initDescriptorSets();
        initMaterials();
        initObjects();
    }

    void Scene::initDescriptorSets() {
        // initialize the global descriptor sets
        m_sceneDescriptorSets.resize(m_rRenderer.getMaximumFramesInFlight());
        m_uboSceneBuffers.resize(m_rRenderer.getMaximumFramesInFlight());

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

    void Scene::initMaterials() {
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

        pipelineConfig.m_renderPass = m_rRenderer.getRenderPass();
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

        texturedPipelineConfigInfo.m_renderPass = m_rRenderer.getRenderPass();
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

    void Scene::initObjects() {
        RenderObject plane{};
        plane.m_pModel = AssetsManager::getModel("Plane");
        plane.m_pMaterial = AssetsManager::getMaterial("DefaultMeshNonTextured");
        plane.m_transform.m_translation = {0.0f, 0.0f, 0.0f};
        plane.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        m_renderObjects.push_back(plane);

        RenderObject star{};
        star.m_pModel = AssetsManager::getModel("Star");
        star.m_pMaterial = AssetsManager::getMaterial("DefaultMeshNonTextured");
        star.m_transform.m_translation = {-0.3f, 0.2f, 0.0f};
        star.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        m_renderObjects.push_back(star);

        RenderObject texturedStar{};
        texturedStar.m_pModel = AssetsManager::getModel("Star");
        texturedStar.m_pMaterial = AssetsManager::getMaterial("DefaultMeshTextured");
        texturedStar.m_transform.m_translation = {0.3f, 0.2f, 0.0f};
        texturedStar.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        texturedStar.m_transform.m_rotation = {180.0f, 0.0f, 0.0f};
        m_renderObjects.push_back(texturedStar);
    }
}