#ifndef IRIS_OBJECTS_HPP
#define IRIS_OBJECTS_HPP

#include "Model.hpp"
#include "Material.hpp"

#include <memory>

namespace iris::graphics{

    struct Transform{
        glm::vec3 m_translation{};
        glm::vec3 m_scale{1.f, 1.f, 1.f};
        glm::vec3 m_rotation{};
    };

    struct GpuObjectData{
        glm::mat4 m_modelMatrix{1.f };
        glm::mat4 m_normalMatrix{1.f };
    };

    struct GpuCameraData{
        glm::mat4 m_view;
        glm::mat4 m_proj;
        glm::mat4 m_viewProj;
    };


    class RenderObject {
    public:
        std::shared_ptr<Model> m_pModel{};
        std::shared_ptr<Material> m_pMaterial{};
        Transform m_transform{};
        GpuObjectData m_gpuObjectData{};

        void update();
    private:
        glm::mat4 modelMatrix();
        glm::mat3 normalMatrix();
    };

    class Camera{
    public:
        Camera();

        Transform m_transform{};
        GpuCameraData m_gpuCameraData{};

        void update(VkExtent2D windowExtent, float dt);
    private:
        glm::mat4 m_viewMatrix = glm::mat4(1.0f);
        glm::mat4 m_projectionMatrix = glm::mat4(1.0f);

        glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 m_up = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);

        float m_speed = 100.0f;
    };

}

#endif //IRIS_OBJECTS_HPP
