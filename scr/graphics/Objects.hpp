#ifndef IRIS_OBJECTS_HPP
#define IRIS_OBJECTS_HPP

#include "Model.hpp"
#include "Material.hpp"

#include <memory>
#include <utility>

namespace iris::graphics{
    struct Transform{
        glm::vec3 m_translation{};
        glm::vec3 m_scale{1.f, 1.f, 1.f};
        glm::vec3 m_rotation{};
    };

    class RenderObject {
    public:
        struct GpuObjectData{
            glm::mat4 m_modelMatrix{1.f };
            glm::mat4 m_normalMatrix{1.f };
        } m_gpuObjectData;

        std::shared_ptr<Model> m_pModel{};
        std::shared_ptr<Model> m_pBoundingBox{};
        std::shared_ptr<Material::MaterialInstance> m_pMaterial{};
        Transform m_transform{};

        void updateInfo(){
            m_gpuObjectData.m_modelMatrix = modelMatrix();
            m_gpuObjectData.m_normalMatrix = normalMatrix();
        }

        std::shared_ptr<Model> setModel(std::shared_ptr<Model> model){
            m_pModel = std::move(model);
            m_pBoundingBox = std::make_shared<Model>(m_pModel->m_rDevice, *m_pModel);
            return m_pModel;
        }
    private:
        glm::mat4 modelMatrix();
        glm::mat3 normalMatrix();

    };

    class Camera{
    public:
        struct GpuCameraData{
            glm::mat4 m_view;
            glm::mat4 m_proj;
            glm::mat4 m_viewProj;
        };

        Camera();

        Transform m_transform{};

        glm::mat4 m_viewMatrix = glm::mat4(1.0f);
        glm::mat4 m_projectionMatrix = glm::mat4(1.0f);

        void update(VkExtent2D windowExtent, float dt);
    private:
        glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 m_up = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);

        float m_speed = 100.0f;
    };

    class PointLight{
    public:
        PointLight(glm::vec3 position, glm::vec3 color);

        struct GpuPointLightData{
            glm::vec3 m_lightPosition;
            alignas(16) glm::vec4 m_lightColor;
        }m_gpuLightData;
    };

    struct GpuSceneData{
        glm::mat4 m_projectionMatrix;
        glm::mat4 m_viewMatrix;
        glm::vec4 m_ambientLightColor; // w is intesity
        int m_numLights;
        PointLight::GpuPointLightData m_lights[10];
    };
}

#endif //IRIS_OBJECTS_HPP
