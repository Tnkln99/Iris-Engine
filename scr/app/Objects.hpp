#ifndef IRIS_OBJECTS_HPP
#define IRIS_OBJECTS_HPP

#include "../graphics/Model.hpp"
#include "../graphics/Material.hpp"

#include <memory>
#include <utility>

namespace iris::app{
    struct Transform{
        glm::vec3 m_translation{};
        glm::vec3 m_scale{1.f, 1.f, 1.f};
        glm::vec3 m_rotation{};
    };

    struct BoundingBox{
        BoundingBox() = default;
        explicit BoundingBox(graphics::Model& model){
            calculateLocalBounds(model);
        };
        bool m_show = false;
        std::shared_ptr<graphics::Model> m_pDebugModel{};
        glm::vec3 m_minLocal{};
        glm::vec3 m_maxLocal{};

        glm::vec3 m_minWorld{};
        glm::vec3 m_maxWorld{};

        void update(float dt, glm::mat4 modelMatrix){
            // Define the 8 corners of the bounding box in local space
            std::vector<glm::vec3> corners = {
                    glm::vec3(m_minLocal.x, m_minLocal.y, m_minLocal.z),
                    glm::vec3(m_maxLocal.x, m_minLocal.y, m_minLocal.z),
                    glm::vec3(m_maxLocal.x, m_maxLocal.y, m_minLocal.z),
                    glm::vec3(m_minLocal.x, m_maxLocal.y, m_minLocal.z),
                    glm::vec3(m_minLocal.x, m_minLocal.y, m_maxLocal.z),
                    glm::vec3(m_maxLocal.x, m_minLocal.y, m_maxLocal.z),
                    glm::vec3(m_maxLocal.x, m_maxLocal.y, m_maxLocal.z),
                    glm::vec3(m_minLocal.x, m_maxLocal.y, m_maxLocal.z)
            };

            // Transform all corners to world space
            for (glm::vec3& corner : corners) {
                glm::vec4 worldSpaceCorner = modelMatrix * glm::vec4(corner, 1.0f);
                corner.x = worldSpaceCorner.x;
                corner.y = worldSpaceCorner.y;
                corner.z = worldSpaceCorner.z;
            }

            // Initialize minWorld and maxWorld to the first corner
            glm::vec3 minWorld = corners[0];
            glm::vec3 maxWorld = corners[0];

            // Find the new min and max extents in world space
            for (const glm::vec3& corner : corners) {
                minWorld.x = std::min(minWorld.x, corner.x);
                minWorld.y = std::min(minWorld.y, corner.y);
                minWorld.z = std::min(minWorld.z, corner.z);

                maxWorld.x = std::max(maxWorld.x, corner.x);
                maxWorld.y = std::max(maxWorld.y, corner.y);
                maxWorld.z = std::max(maxWorld.z, corner.z);
            }

            // Update the world-space bounding box
            m_minWorld = minWorld;
            m_maxWorld = maxWorld;
        }

        bool intersects(glm::vec3 min, glm::vec3 max){
            // Check for overlap along the X axis
            if (m_maxWorld.x < min.x || m_minWorld.x > max.x) {
                return false; // No overlap, the AABB is completely to the left or right of the cell
            }

            // Check for overlap along the Y axis
            if (m_maxWorld.y < min.y || m_minWorld.y > max.y) {
                return false; // No overlap, the AABB is completely above or below the cell
            }

            // Check for overlap along the Z axis
            if (m_maxWorld.z < min.z || m_minWorld.z > max.z) {
                return false; // No overlap, the AABB is completely in front of or behind the cell
            }

            // If we get here, there's overlap along all three axes, so the AABB intersects the cell
            return true;
        }

        bool rayIntersectsBox(const glm::vec3& rayOrigin, const glm::vec3& rayDir) {
            glm::vec3 tMin = (m_minWorld - rayOrigin) / rayDir;
            glm::vec3 tMax = (m_maxWorld - rayOrigin) / rayDir;
            glm::vec3 t1 = min(tMin, tMax);
            glm::vec3 t2 = max(tMin, tMax);
            float tNear = std::max(std::max(t1.x, t1.y), t1.z);
            float tFar = std::min(std::min(t2.x, t2.y), t2.z);
            return tNear <= tFar;
        }

        void calculateLocalBounds(graphics::Model& model){
            graphics::Model::Builder builder{};
            glm::vec3 min = model.m_vertices[0].m_position, max = model.m_vertices[0].m_position;
            for (const auto& vertex : model.m_vertices) {
                min.x = std::min(min.x, vertex.m_position.x);
                min.y = std::min(min.y, vertex.m_position.y);
                min.z = std::min(min.z, vertex.m_position.z);

                max.x = std::max(max.x, vertex.m_position.x);
                max.y = std::max(max.y, vertex.m_position.y);
                max.z = std::max(max.z, vertex.m_position.z);
            }

            m_minLocal = min;
            m_maxLocal = max;

            if (m_maxLocal.x - m_minLocal.x == 0){
                m_maxLocal.x += 0.1f;
                m_minLocal.x -= 0.1f;

            }
            if (m_maxLocal.y - m_minLocal.y == 0){
                m_maxLocal.y += 0.1f;
                m_minLocal.y -= 0.1f;
            }
            if (m_maxLocal.z - m_minLocal.z == 0){
                m_maxLocal.z += 0.1f;
                m_minLocal.z -= 0.1f;
            }

            builder.m_vertices = {
                    // Bottom face
                    graphics::Model::Vertex(glm::vec3(m_minLocal.x, m_minLocal.y, m_minLocal.z), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
                    graphics::Model::Vertex(glm::vec3(m_maxLocal.x, m_minLocal.y, m_minLocal.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
                    graphics::Model::Vertex(glm::vec3(m_maxLocal.x, m_maxLocal.y, m_minLocal.z), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
                    graphics::Model::Vertex(glm::vec3(m_minLocal.x, m_maxLocal.y, m_minLocal.z), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),

                    // Top face
                    graphics::Model::Vertex(glm::vec3(m_minLocal.x, m_minLocal.y, m_maxLocal.z), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
                    graphics::Model::Vertex(glm::vec3(m_maxLocal.x, m_minLocal.y, m_maxLocal.z), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
                    graphics::Model::Vertex(glm::vec3(m_maxLocal.x, m_maxLocal.y, m_maxLocal.z), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
                    graphics::Model::Vertex(glm::vec3(m_minLocal.x, m_maxLocal.y, m_maxLocal.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
            };

            builder.m_indices = {
                    // Bottom face
                    0, 1, 2, 2, 3, 0,
                    // Top face
                    4, 5, 6, 6, 7, 4,
                    // Sides
                    0, 4, 5, 5, 1, 0,
                    1, 5, 6, 6, 2, 1,
                    2, 6, 7, 7, 3, 2,
                    3, 7, 4, 4, 0, 3
            };

            m_pDebugModel = std::make_shared<graphics::Model>(model.m_rDevice, builder);
        }
    };
    class RenderObject {
    public:
        struct GpuObjectData{
            glm::mat4 m_modelMatrix{1.f };
            glm::mat4 m_normalMatrix{1.f };
        } m_gpuObjectData;

        std::shared_ptr<graphics::Material::MaterialInstance> m_pMaterialInstance{};
        Transform m_transform{};

        [[nodiscard]] std::shared_ptr<graphics::Model> getModel() const { return m_pModel; }
        [[nodiscard]] BoundingBox& getBoundingBox() { return m_boundingBox; }

        void updateInfo(){
            m_gpuObjectData.m_modelMatrix = modelMatrix();
            m_gpuObjectData.m_normalMatrix = normalMatrix();
        }

        void setModel(std::shared_ptr<graphics::Model> model){
            m_pModel = std::move(model);
            m_boundingBox = BoundingBox(*m_pModel);
        }
        glm::mat4 modelMatrix();
        glm::mat3 normalMatrix();
        BoundingBox m_boundingBox{};

        std::string m_name = "default";
        bool m_bRender = true;
    private:

        std::shared_ptr<graphics::Model> m_pModel{};
    };

    class Camera{
    public:
        struct GpuCameraData{
            glm::mat4 m_view;
            glm::mat4 m_proj;
            glm::mat4 m_viewProj;
        };
        explicit Camera(graphics::Window& window);

        [[nodiscard]] glm::mat4 getViewMatrix() const { return m_viewMatrix; }
        [[nodiscard]] glm::mat4 getProjectionMatrix() const { return m_projectionMatrix; }
        [[nodiscard]] Transform getTransform() const { return m_transform; }
        glm::vec3 getCameraRay(double x, double y);

        Transform m_transform{};

        glm::mat4 m_viewMatrix = glm::mat4(1.0f);
        glm::mat4 m_projectionMatrix = glm::mat4(1.0f);

        void update(float dt);
        glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    private:
        graphics::Window& m_rWindow;
        glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);

        float m_yaw = -90.0f;
        float m_pitch = 0.0f;
        float m_lastX = 0;
        float m_lastY = 0;
        bool m_firstMouse = true;

        float m_speed = 100.0f;

        void processMouseMovement(float xOffset, float yOffset);
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
