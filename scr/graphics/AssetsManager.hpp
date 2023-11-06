#ifndef IRIS_ASSETSMANAGER_HPP
#define IRIS_ASSETSMANAGER_HPP

#include "Model.hpp"
#include "Material.hpp"

#include <memory>
#include <unordered_map>
#include <string>

namespace iris::graphics{
    class AssetsManager {
    public:
        AssetsManager(const AssetsManager&) = delete;
        AssetsManager& operator=(const AssetsManager&) = delete;

        static void loadModel(Device& device, const std::string& name, const std::string& path);
        static std::shared_ptr<Model> getModel(const std::string& name);

        static void loadMaterial(Device& device, const std::string& name, const std::shared_ptr<Pipeline>& pipeline, VkPipelineLayout layout, VkDescriptorSet texture = VK_NULL_HANDLE);
        static std::shared_ptr<Material> getMaterial(const std::string& name);

        static void loadTexture(Device& device, const std::string& name, const std::string& path);
        static std::shared_ptr<Texture> getTexture(const std::string& name);

        static void clear(Device& device);
    private:
        static std::unordered_map<std::string, std::shared_ptr<Model>> m_sModels;
        static std::unordered_map<std::string, std::shared_ptr<Material>> m_sMaterials;
        static std::unordered_map<std::string, std::shared_ptr<Texture>> m_sTextures;
    };
}




#endif //IRIS_ASSETSMANAGER_HPP
