#include "AssetsManager.hpp"
#include "Initializers.hpp"


namespace iris::graphics{
    std::unordered_map<std::string, std::shared_ptr<Model>> AssetsManager::m_models = std::unordered_map<std::string, std::shared_ptr<Model>>{};
    std::unordered_map<std::string, std::shared_ptr<Material>> AssetsManager::m_materials = std::unordered_map<std::string, std::shared_ptr<Material>>{};
    std::unordered_map<std::string, std::shared_ptr<Texture>> AssetsManager::m_textures = std::unordered_map<std::string, std::shared_ptr<Texture>>{};

    void AssetsManager::clear(Device& device) {
        m_models.clear();
        for(auto & material : m_materials){
            vkDestroyPipelineLayout(device.getDevice(), material.second->getPipeLineLayout(), nullptr);
        }

        m_materials.clear();

        for(auto & texture : m_textures){
            vkDestroyImageView(device.getDevice(), texture.second->m_imageView, nullptr);
            vkDestroyImage(device.getDevice(), texture.second->m_image.m_image, nullptr);
        }
        m_textures.clear();
    }

    void AssetsManager::loadModel(Device& device, const std::string& name, const std::string& path){
        if(m_models.find(name) != m_models.end()){
            std::cout << "Model " << name << " already loaded!" << std::endl;
            return;
        }

        m_models[name] = Model::createModelFromFile(device, path);
    }

    std::shared_ptr<Model> AssetsManager::getModel(const std::string &name) {
        if(m_models.find(name) == m_models.end()){
            std::cout << "Model " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_models[name];
    }

    void AssetsManager::loadMaterial(Device& device, const std::string &name, const std::shared_ptr<Pipeline>& pipeline, VkPipelineLayout layout,
                                     VkDescriptorSet texture) {
        if(m_materials.find(name) != m_materials.end()){
            std::cout << "Material " << name << " already loaded!" << std::endl;
            return;
        }
        m_materials[name] = std::make_shared<Material>(device, name, pipeline, layout, texture);
    }

    std::shared_ptr<Material> AssetsManager::getMaterial(const std::string &name) {
        if(m_materials.find(name) == m_materials.end()){
            std::cout << "Material " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_materials[name];
    }

    void AssetsManager::loadTexture(Device &device, const std::string &name, const std::string &path) {
        if(m_textures.find(name) != m_textures.end()){
            std::cout << "Texture " << name << " already loaded!" << std::endl;
            return;
        }

        std::shared_ptr<Texture> textureToLoad = std::make_shared<Texture>();
        AllocatedImage image = device.loadTexture(path);

        textureToLoad->m_name = name;
        textureToLoad->m_image = image;

        VkImageViewCreateInfo imageInfo = Initializers::createImageViewInfo(VK_FORMAT_R8G8B8A8_SRGB, textureToLoad->m_image.m_image, VK_IMAGE_ASPECT_COLOR_BIT);
        vkCreateImageView(device.getDevice(), &imageInfo, nullptr, &textureToLoad->m_imageView);

        m_textures[name] = textureToLoad;
    }

    std::shared_ptr<Texture> AssetsManager::getTexture(const std::string &name) {
        if(m_textures.find(name) == m_textures.end()){
            std::cout << "Texture " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_textures[name];
    }
}