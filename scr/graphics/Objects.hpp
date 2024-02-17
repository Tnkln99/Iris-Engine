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

    struct BoundingBox{
        BoundingBox() = default;
        explicit BoundingBox(Model& model){
            calculateLocalBounds(model);
        };
        bool m_show = true;
        std::shared_ptr<Model> m_pDebugModel{};
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

        bool rayIntersectsBox(const glm::vec3& rayOrigin, const glm::vec3& rayDir) {
            float tMin = 0.0f;
            float tMax = std::numeric_limits<float>::max();
            const float EPSILON = 1e-8f; // A small epsilon value to handle floating-point errors

            for (int i = 0; i < 3; ++i) {
                if (std::abs(rayDir[i]) < EPSILON) {
                    // Ray is parallel to slab. No hit if origin not within slab
                    if (rayOrigin[i] < m_minWorld[i] || rayOrigin[i] > m_maxWorld[i]) return false;
                } else {
                    float invD = 1.0f / rayDir[i];
                    float t0 = (m_minWorld[i] - rayOrigin[i]) * invD;
                    float t1 = (m_maxWorld[i] - rayOrigin[i]) * invD;
                    if (invD < 0.0f) std::swap(t0, t1);
                    tMin = std::max(tMin, t0);
                    tMax = std::min(tMax, t1);
                    if (tMax <= tMin) return false;
                }
            }

            return true;
        }

        void calculateLocalBounds(Model& model){
            Model::Builder builder{};
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
                    Model::Vertex(glm::vec3(m_minLocal.x, m_minLocal.y, m_minLocal.z), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
                    Model::Vertex(glm::vec3(m_maxLocal.x, m_minLocal.y, m_minLocal.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
                    Model::Vertex(glm::vec3(m_maxLocal.x, m_maxLocal.y, m_minLocal.z), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
                    Model::Vertex(glm::vec3(m_minLocal.x, m_maxLocal.y, m_minLocal.z), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),

                    // Top face
                    Model::Vertex(glm::vec3(m_minLocal.x, m_minLocal.y, m_maxLocal.z), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
                    Model::Vertex(glm::vec3(m_maxLocal.x, m_minLocal.y, m_maxLocal.z), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
                    Model::Vertex(glm::vec3(m_maxLocal.x, m_maxLocal.y, m_maxLocal.z), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
                    Model::Vertex(glm::vec3(m_minLocal.x, m_maxLocal.y, m_maxLocal.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
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

            m_pDebugModel = std::make_shared<Model>(model.m_rDevice, builder);
        }
    };

    class RenderObject {
    public:
        struct GpuObjectData{
            glm::mat4 m_modelMatrix{1.f };
            glm::mat4 m_normalMatrix{1.f };
        } m_gpuObjectData;

        std::shared_ptr<Material::MaterialInstance> m_pMaterialInstance{};
        Transform m_transform{};

        [[nodiscard]] std::shared_ptr<Model> getModel() const { return m_pModel; }
        [[nodiscard]] BoundingBox& getBoundingBox() { return m_boundingBox; }

        void updateInfo(){
            m_gpuObjectData.m_modelMatrix = modelMatrix();
            m_gpuObjectData.m_normalMatrix = normalMatrix();
        }

        void setModel(std::shared_ptr<Model> model){
            m_pModel = std::move(model);
            m_boundingBox = BoundingBox(*m_pModel);
        }
        glm::mat4 modelMatrix();
        glm::mat3 normalMatrix();
    private:

        std::shared_ptr<Model> m_pModel{};
        BoundingBox m_boundingBox{};
    };

    class Camera{
    public:
        struct GpuCameraData{
            glm::mat4 m_view;
            glm::mat4 m_proj;
            glm::mat4 m_viewProj;
        };
        explicit Camera(Window& window);

        [[nodiscard]] glm::mat4 getViewMatrix() const { return m_viewMatrix; }
        [[nodiscard]] glm::mat4 getProjectionMatrix() const { return m_projectionMatrix; }
        [[nodiscard]] Transform getTransform() const { return m_transform; }
        glm::vec3 getCameraRay(double x, double y);
        glm::vec3 screenPointToRayOrigin(glm::vec3 rayDir);

        Transform m_transform{};

        glm::mat4 m_viewMatrix = glm::mat4(1.0f);
        glm::mat4 m_projectionMatrix = glm::mat4(1.0f);

        void update(float dt);
    private:
        Window& m_rWindow;
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
