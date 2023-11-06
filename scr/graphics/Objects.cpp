#include "Objects.hpp"
#include "Window.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace iris::graphics{

    glm::mat4 RenderObject::modelMatrix() {
        const float c3 = glm::cos(m_transform.m_rotation.z);
        const float s3 = glm::sin(m_transform.m_rotation.z);
        const float c2 = glm::cos(m_transform.m_rotation.x);
        const float s2 = glm::sin(m_transform.m_rotation.x);
        const float c1 = glm::cos(m_transform.m_rotation.y);
        const float s1 = glm::sin(m_transform.m_rotation.y);
        return glm::mat4{
                {
                        m_transform.m_scale.x * (c1 * c3 + s1 * s2 * s3),
                                                     m_transform.m_scale.x * (c2 * s3),
                                                                                  m_transform.m_scale.x * (c1 * s2 * s3 - c3 * s1),
                                                                                                               0.0f,
                },
                {
                        m_transform.m_scale.y * (c3 * s1 * s2 - c1 * s3),
                                                     m_transform.m_scale.y * (c2 * c3),
                                                                                  m_transform.m_scale.y * (c1 * c3 * s2 + s1 * s3),
                                                                                                               0.0f,
                },
                {
                        m_transform.m_scale.z * (c2 * s1),
                                                     m_transform.m_scale.z * (-s2),
                                                                                  m_transform.m_scale.z * (c1 * c2),
                                                                                                               0.0f,
                },
                {       m_transform.m_translation.x, m_transform.m_translation.y, m_transform.m_translation.z, 1.0f}};
    }

    glm::mat3 RenderObject::normalMatrix() {
        const float c3 = glm::cos(m_transform.m_rotation.z);
        const float s3 = glm::sin(m_transform.m_rotation.z);
        const float c2 = glm::cos(m_transform.m_rotation.x);
        const float s2 = glm::sin(m_transform.m_rotation.x);
        const float c1 = glm::cos(m_transform.m_rotation.y);
        const float s1 = glm::sin(m_transform.m_rotation.y);
        const glm::vec3 invScale = 1.0f / m_transform.m_scale;

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
        m_gpuObjectData.m_modelMatrix = modelMatrix();
        m_gpuObjectData.m_normalMatrix = normalMatrix();
    }

    ////////////////////////////////////////////////////////////////////////////

    Camera::Camera() {
        m_transform.m_translation = {1, 2, 1};
    }

    void Camera::update(VkExtent2D windowExtent, float dt) {
        if (Window::m_sKeyInfo.m_key == GLFW_KEY_D && Window::m_sKeyInfo.m_action != 0){
            m_transform.m_translation = glm::rotate(
                    m_transform.m_translation,
                    glm::radians(m_speed) * dt,
                    m_up);
        }
        if (Window::m_sKeyInfo.m_key == GLFW_KEY_A && Window::m_sKeyInfo.m_action != 0){
            m_transform.m_translation = glm::rotate(
                    m_transform.m_translation,
                    -glm::radians(m_speed) * dt,
                    m_up);
        }



        m_viewMatrix = glm::lookAt(m_transform.m_translation,
                                   glm::vec3(0,0,0),
                                   m_up);
        m_projectionMatrix = glm::perspective(glm::radians(45.0f), (float)windowExtent.width / (float)windowExtent.height, 0.1f, 100.0f);

        m_gpuCameraData.m_proj = m_projectionMatrix;
        m_gpuCameraData.m_view = m_viewMatrix;
        m_gpuCameraData.m_viewProj = m_projectionMatrix * m_viewMatrix;
    }
}