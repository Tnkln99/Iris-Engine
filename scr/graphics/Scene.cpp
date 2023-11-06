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
        for (auto & uboBuffer : m_UboCameraBuffers)
        {
            m_rDevice.destroyBuffer(uboBuffer);
        }
        m_RenderObjects.clear();
    }

    void Scene::draw() {
        VkCommandBuffer cmd = m_rRenderer.beginFrame();

        m_rDevice.copyToBuffer(&m_Camera.m_GpuCameraData,
                               m_UboCameraBuffers[m_rRenderer.getCurrentFrame()],
                               sizeof(GpuCameraData));
        m_Camera.update(m_rRenderer.getSwapchainExtent(), utils::Timer::getDeltaTime());


        std::shared_ptr<Model> lastModel = nullptr;
        std::shared_ptr<Material> lastMaterial = nullptr;
        for(auto & renderObject : m_RenderObjects){
            renderObject.update();
            auto material = renderObject.m_Material;
            if(lastMaterial != material){
                material->getPipeline()->bind(cmd);

                vkCmdBindDescriptorSets(
                        cmd,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        material->getPipeLineLayout(),
                        0,
                        1,
                        &m_CameraDescriptorSets[m_rRenderer.getCurrentFrame()],
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
                    sizeof(GpuObjectData),
                    &renderObject.m_GpuObjectData);


            if(lastModel != renderObject.m_Model){
                renderObject.m_Model->bind(cmd);
                lastModel = renderObject.m_Model;
            }
            renderObject.m_Model->draw(cmd);
        }


        m_rRenderer.endFrame(cmd);
    }

    void Scene::loadScene() {
        loadModels();
        loadImages();
        initDescriptorSets();
        initMaterials();
        initObjects();
    }

    void Scene::loadModels() {
        AssetsManager::loadModel(m_rDevice, "Star", "../assets/models/Star/Star.obj");
        AssetsManager::loadModel(m_rDevice, "Plane", "../assets/models/Plane/Plane.obj");
    }

    void Scene::loadImages() {
        AssetsManager::loadTexture(m_rDevice, "StarDiffuse", "../assets/models/Star/Diffuse.png");
    }

    void Scene::initDescriptorSets() {
        // initialize the global descriptor sets
        m_CameraDescriptorSets.resize(m_rRenderer.getMaximumFramesInFlight());
        m_UboCameraBuffers.resize(m_rRenderer.getMaximumFramesInFlight());

        m_GlobalPool = DescriptorPool::Builder(m_rDevice)
                .setMaxSets(10)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10)
                .build();

        for(auto & uboBuffer : m_UboCameraBuffers)
        {
            uboBuffer = m_rDevice.createBuffer(sizeof(GpuCameraData),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                               VMA_MEMORY_USAGE_CPU_TO_GPU);
        }

        m_GlobalSetLayout = DescriptorSetLayout::Builder(m_rDevice)
                        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .build();

        for (int i = 0; i < m_CameraDescriptorSets.size(); i++)
        {
            VkDescriptorBufferInfo cameraInfo;
            cameraInfo.buffer = m_UboCameraBuffers[i].buffer;
            cameraInfo.offset = 0;
            cameraInfo.range = sizeof(GpuCameraData);

            DescriptorWriter(*m_GlobalSetLayout, *m_GlobalPool)
                    .writeBuffer(0, &cameraInfo)
                    .build(m_CameraDescriptorSets[i]);
        }

        m_SingleTexturedSetLayout = DescriptorSetLayout::Builder(m_rDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();
    }

    void Scene::initMaterials() {
        // non textured pipeline
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Initializers::createPipelineLayoutInfo();
        const std::vector descriptorSetLayouts{m_GlobalSetLayout->getDescriptorSetLayout()};

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(GpuObjectData);

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

        pipelineConfig.renderPass = m_rRenderer.getRenderPass();
        pipelineConfig.pipelineLayout = defaultPipelineLayout;

        auto defaultPipeline = std::make_shared<Pipeline>(
                m_rDevice,
                "../shaders/Default.vert.spv",
                "../shaders/Default.frag.spv",
                pipelineConfig);

        AssetsManager::loadMaterial(m_rDevice, "DefaultMeshNonTextured", defaultPipeline, defaultPipelineLayout);

        // textured pipeline
        VkPipelineLayoutCreateInfo texturedPipelineLayoutCreateInfo = Initializers::createPipelineLayoutInfo();
        const std::vector texturedDescriptorSetLayouts{m_GlobalSetLayout->getDescriptorSetLayout(), m_SingleTexturedSetLayout->getDescriptorSetLayout()};

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

        texturedPipelineConfigInfo.renderPass = m_rRenderer.getRenderPass();
        texturedPipelineConfigInfo.pipelineLayout = texturedPipelineLayout;

        auto texturedPipeline = std::make_shared<Pipeline>(
                m_rDevice,
                "../shaders/Default.vert.spv",
                "../shaders/DefaultTextured.frag.spv",
                texturedPipelineConfigInfo);

        AssetsManager::loadMaterial(m_rDevice, "DefaultMeshTextured", texturedPipeline, texturedPipelineLayout);
        auto mat = AssetsManager::getMaterial("DefaultMeshTextured");
        mat->setTexture(AssetsManager::getTexture("StarDiffuse"), *m_GlobalPool, *m_SingleTexturedSetLayout);
    }

    void Scene::initObjects() {
        RenderObject plane{};
        plane.m_Model = AssetsManager::getModel("Plane");
        plane.m_Material = AssetsManager::getMaterial("DefaultMeshNonTextured");
        plane.m_Transform.translation = {0.0f, 0.0f, 0.0f};
        plane.m_Transform.scale = {0.5f, 0.5f, 0.5f};
        m_RenderObjects.push_back(plane);

        RenderObject star{};
        star.m_Model = AssetsManager::getModel("Star");
        star.m_Material = AssetsManager::getMaterial("DefaultMeshNonTextured");
        star.m_Transform.translation = {-0.3f, 0.2f, 0.0f};
        star.m_Transform.scale = {0.5f, 0.5f, 0.5f};
        m_RenderObjects.push_back(star);

        RenderObject texturedStar{};
        texturedStar.m_Model = AssetsManager::getModel("Star");
        texturedStar.m_Material = AssetsManager::getMaterial("DefaultMeshTextured");
        texturedStar.m_Transform.translation = {0.3f, 0.2f, 0.0f};
        texturedStar.m_Transform.scale = {0.5f, 0.5f, 0.5f};
        m_RenderObjects.push_back(texturedStar);
    }
}