#ifndef IRIS_OBJECTS_HPP
#define IRIS_OBJECTS_HPP

#include "Model.hpp"

#include <memory>

namespace iris::graphics{

    struct Transform{
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};
    };

    struct GpuObjectData{
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };

    struct GpuCameraData{
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 viewProj;
    };


    class RenderObject {
    public:
        std::shared_ptr<Model> m_Model{};
        Transform m_Transform{};
        GpuObjectData m_GpuObjectData{};

        void update();
    private:
        glm::mat4 modelMatrix();
        glm::mat3 normalMatrix();
    };

    class Camera{
    public:
        Camera();

        Transform m_Transform{};
        GpuCameraData m_GpuCameraData{};

        void update(VkExtent2D windowExtent, float dt);
    private:
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

        glm::vec3 m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 m_Up = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);

        float m_Yaw   = -90.0f;
        float m_Pitch =  0.0f;
        float m_Fov   =  45.0f;

        float m_Speed = 100.0f;
    };

}

#endif //IRIS_OBJECTS_HPP
