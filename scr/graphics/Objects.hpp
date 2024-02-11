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

        [[nodiscard]] std::pair<glm::vec3, glm::vec3> getWorldPositions(glm::mat4 modelMatrix) const{
            glm::vec3 min = modelMatrix * glm::vec4(m_minLocal, 1.0f);
            glm::vec3 max = modelMatrix * glm::vec4(m_maxLocal, 1.0f);

            std::pair<glm::vec3, glm::vec3> minMax{min, max};
            return minMax;
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

            builder.m_vertices = {
                    // Bottom face
                    Model::Vertex(glm::vec3(min.x, min.y, min.z), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
                    Model::Vertex(glm::vec3(max.x, min.y, min.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
                    Model::Vertex(glm::vec3(max.x, max.y, min.z), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
                    Model::Vertex(glm::vec3(min.x, max.y, min.z), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),

                    // Top face
                    Model::Vertex(glm::vec3(min.x, min.y, max.z), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
                    Model::Vertex(glm::vec3(max.x, min.y, max.z), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
                    Model::Vertex(glm::vec3(max.x, max.y, max.z), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
                    Model::Vertex(glm::vec3(min.x, max.y, max.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
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
        [[nodiscard]] BoundingBox getBoundingBox() const { return m_boundingBox; }

        void updateInfo(){
            m_gpuObjectData.m_modelMatrix = modelMatrix();
            m_gpuObjectData.m_normalMatrix = normalMatrix();
        }

        void setModel(std::shared_ptr<Model> model){
            m_pModel = std::move(model);
            m_boundingBox = BoundingBox(*m_pModel);
        }
    private:
        glm::mat4 modelMatrix();
        glm::mat3 normalMatrix();

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
