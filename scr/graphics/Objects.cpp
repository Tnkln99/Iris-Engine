#include "Objects.hpp"
#include "Window.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace iris::graphics{

    glm::mat4 RenderObject::modelMatrix() {
        const float c3 = glm::cos(m_Transform.rotation.z);
        const float s3 = glm::sin(m_Transform.rotation.z);
        const float c2 = glm::cos(m_Transform.rotation.x);
        const float s2 = glm::sin(m_Transform.rotation.x);
        const float c1 = glm::cos(m_Transform.rotation.y);
        const float s1 = glm::sin(m_Transform.rotation.y);
        return glm::mat4{
                {
                        m_Transform.scale.x * (c1 * c3 + s1 * s2 * s3),
                                m_Transform.scale.x * (c2 * s3),
                                               m_Transform.scale.x * (c1 * s2 * s3 - c3 * s1),
                                                              0.0f,
                },
                {
                        m_Transform.scale.y * (c3 * s1 * s2 - c1 * s3),
                                m_Transform.scale.y * (c2 * c3),
                                               m_Transform.scale.y * (c1 * c3 * s2 + s1 * s3),
                                                              0.0f,
                },
                {
                        m_Transform.scale.z * (c2 * s1),
                                m_Transform.scale.z * (-s2),
                                               m_Transform.scale.z * (c1 * c2),
                                                              0.0f,
                },
                {m_Transform.translation.x, m_Transform.translation.y, m_Transform.translation.z, 1.0f}};
    }

    glm::mat3 RenderObject::normalMatrix() {
        const float c3 = glm::cos(m_Transform.rotation.z);
        const float s3 = glm::sin(m_Transform.rotation.z);
        const float c2 = glm::cos(m_Transform.rotation.x);
        const float s2 = glm::sin(m_Transform.rotation.x);
        const float c1 = glm::cos(m_Transform.rotation.y);
        const float s1 = glm::sin(m_Transform.rotation.y);
        const glm::vec3 invScale = 1.0f / m_Transform.scale;

        return glm::mat3{
                {
                        invScale.x * (c1 * c3 + s1 * s2 * s3),
                        invScale.x * (c2 * s3),
                        invScale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                        invScale.y * (c3 * s1 * s2 - c1 * s3),
                        invScale.y * (c2 * c3),
                        invScale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                        invScale.z * (c2 * s1),
                        invScale.z * (-s2),
                        invScale.z * (c1 * c2),
                },
        };
    }

    void RenderObject::update() {
        m_GpuObjectData.modelMatrix = modelMatrix();
        m_GpuObjectData.normalMatrix = normalMatrix();
    }

    ////////////////////////////////////////////////////////////////////////////

    Camera::Camera() {
        m_Transform.translation = {1,1.3,1};
    }

    void Camera::update(VkExtent2D windowExtent, float dt) {
        if (Window::m_sKeyInfo.key == GLFW_KEY_D && Window::m_sKeyInfo.action != 0){
            m_Transform.translation = glm::rotate(
                    m_Transform.translation,
                    glm::radians(m_Speed) * dt,
                    m_Up);
        }
        if (Window::m_sKeyInfo.key == GLFW_KEY_A && Window::m_sKeyInfo.action != 0){
            m_Transform.translation = glm::rotate(
                    m_Transform.translation,
                    -glm::radians(m_Speed) * dt,
                    m_Up);
        }



        m_ViewMatrix = glm::lookAt(m_Transform.translation,
                                   glm::vec3(0,0,0),
                                   m_Up);
        m_ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)windowExtent.width / (float)windowExtent.height, 0.1f, 100.0f);

        m_GpuCameraData.proj = m_ProjectionMatrix;
        m_GpuCameraData.view = m_ViewMatrix;
        m_GpuCameraData.viewProj = m_ProjectionMatrix * m_ViewMatrix;
    }
}