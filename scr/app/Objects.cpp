#include "Objects.hpp"
#include "../graphics/Window.hpp"
#include "../scenes/Scene2DNav.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace iris::app{

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


    ////////////////////////////////////////////////////////////////////////////

    Camera::Camera(graphics::Window& window) : m_rWindow{window}{
        m_transform.m_translation = {0, 0.0f, 60};
        m_viewMatrix = glm::lookAt(m_transform.m_translation,
                                   m_transform.m_translation + m_front,
                                   m_up);
        m_projectionMatrix = glm::perspective(glm::radians(45.0f),
                                              (float) m_rWindow.getWidth() / (float) m_rWindow.getHeight(), 0.1f,
                                              1000.0f);
    }

    glm::vec3 Camera::getCameraRay(double mouseX, double mouseY) {
        float x = (2.0f * mouseX) / m_rWindow.getWidth() - 1.0f;
        float y = (2.0f * mouseY) / m_rWindow.getHeight() - 1.0f;
        float z = 1.0f;
        glm::vec3 ray_nds = glm::vec3(x, y, z);

        glm::vec4 ray_clip = glm::vec4(glm::vec2(ray_nds.x, ray_nds.y), -1.0, 1.0);

        glm::vec4 ray_eye = inverse(m_projectionMatrix) * ray_clip;
        ray_eye = glm::vec4(glm::vec2(ray_eye.x,ray_eye.y), -1.0, 0.0);

        glm::vec3 ray_wor = glm::vec3(inverse(m_viewMatrix) * ray_eye);
        ray_wor = glm::normalize(ray_wor);

        return ray_wor;
    }

    void Camera::update(float dt) {
        // Assuming m_speed controls movement speed and m_rotationSpeed controls rotation speed.
        float movementSpeed = m_speed * dt;
        float rotationSpeed = glm::radians(m_speed) * dt;

        // Direction vector from camera to target for forward/backward movement.
        glm::vec3 direction = glm::normalize(m_target - m_transform.m_translation);
        glm::vec3 right = glm::normalize(glm::cross(direction, m_up)); // Right vector for strafing.

        if (graphics::Window::m_sKeyInfo.m_key == GLFW_KEY_W && graphics::Window::m_sKeyInfo.m_action != GLFW_RELEASE) {
            m_transform.m_translation += direction * movementSpeed;
            m_target += direction * movementSpeed;
        }
        if (graphics::Window::m_sKeyInfo.m_key == GLFW_KEY_S && graphics::Window::m_sKeyInfo.m_action != GLFW_RELEASE) {
            m_transform.m_translation -= direction * movementSpeed;
            m_target -= direction * movementSpeed;
        }


        if (graphics::Window::m_sKeyInfo.m_key == GLFW_KEY_A && graphics::Window::m_sKeyInfo.m_action != GLFW_RELEASE) {
            m_transform.m_translation -= right * movementSpeed;
            m_target -= right * movementSpeed;
        }
        if (graphics::Window::m_sKeyInfo.m_key == GLFW_KEY_D && graphics::Window::m_sKeyInfo.m_action != GLFW_RELEASE) {
            m_transform.m_translation += right * movementSpeed;
            m_target += right * movementSpeed;
        }

        auto& mouseInfo = graphics::Window::m_sMouseInfo;


        float xOffset = mouseInfo.m_xPos - m_lastX;
        float yOffset = -(m_lastY - mouseInfo.m_yPos); // Reversed since y-coordinates range from bottom to top

        m_lastX = mouseInfo.m_xPos;
        m_lastY = mouseInfo.m_yPos;

        if(graphics::Window::m_sKeyInfo.m_key == GLFW_KEY_SPACE && graphics::Window::m_sKeyInfo.m_action != GLFW_RELEASE){
            processMouseMovement(xOffset, yOffset);
        }


        m_viewMatrix = glm::lookAt(m_transform.m_translation,
                                   m_target,
                                   m_up);
        m_projectionMatrix = glm::perspective(glm::radians(45.0f),
                                              (float) m_rWindow.getWidth() / (float) m_rWindow.getHeight(), 1.0f,
                                              300.0f);
    }

    void Camera::processMouseMovement(float xOffset, float yOffset) {
        if (m_firstMouse) {
            m_lastX = graphics::Window::m_sMouseInfo.m_xPos;
            m_lastY = graphics::Window::m_sMouseInfo.m_yPos;
            m_firstMouse = false;
        }

        float sensitivity = 0.2f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        m_yaw += xOffset;
        m_pitch += yOffset;

        if (m_pitch > 89.0f) m_pitch = 89.0f;
        if (m_pitch < -89.0f) m_pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);
        m_target = m_transform.m_translation + m_front;
    }


    PointLight::PointLight(glm::vec3 position, glm::vec3 color) {
        m_gpuLightData.m_lightPosition = position;
        m_gpuLightData.m_lightColor = glm::vec4(color, 1.0f);
    }
}