#include "AssetsManager.hpp"
#include "Initializers.hpp"


namespace iris::graphics{
    std::unordered_map<std::string, std::shared_ptr<Model>> AssetsManager::m_sModels = std::unordered_map<std::string, std::shared_ptr<Model>>{};
    std::unordered_map<std::string, std::shared_ptr<Material>> AssetsManager::m_sMaterials = std::unordered_map<std::string, std::shared_ptr<Material>>{};
    std::unordered_map<std::string, std::shared_ptr<Texture>> AssetsManager::m_sTextures = std::unordered_map<std::string, std::shared_ptr<Texture>>{};

    void AssetsManager::clear(Device& device) {
        m_sModels.clear();
        for(auto & material : m_sMaterials){
            vkDestroyPipelineLayout(device.getDevice(), material.second->getPipeLineLayout(), nullptr);
        }

        m_sMaterials.clear();

        for(auto & texture : m_sTextures){
            vkDestroyImageView(device.getDevice(), texture.second->m_imageView, nullptr);
            device.destroyImage(texture.second->m_allocatedImage);
        }
        m_sTextures.clear();
    }

    void AssetsManager::loadModel(Device& device, const std::string& name, const std::string& path){
        if(m_sModels.find(name) != m_sModels.end()){
            std::cout << "Model " << name << " already loaded!" << std::endl;
            return;
        }

        m_sModels[name] = Model::createModelFromFile(device, path);
    }

    std::shared_ptr<Model> AssetsManager::getModel(const std::string &name) {
        if(m_sModels.find(name) == m_sModels.end()){
            std::cout << "Model " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_sModels[name];
    }

    void AssetsManager::loadMaterial(Device& device, const std::string &name, const std::shared_ptr<Pipeline>& pipeline, VkPipelineLayout layout,
                                     VkDescriptorSet texture) {
        if(m_sMaterials.find(name) != m_sMaterials.end()){
            std::cout << "Material " << name << " already loaded!" << std::endl;
            return;
        }
        m_sMaterials[name] = std::make_shared<Material>(device, name, pipeline, layout, texture);
    }

    std::shared_ptr<Material> AssetsManager::getMaterial(const std::string &name) {
        if(m_sMaterials.find(name) == m_sMaterials.end()){
            std::cout << "Material " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_sMaterials[name];
    }

    void AssetsManager::loadTexture(Device &device, const std::string &name, const std::string &path) {
        if(m_sTextures.find(name) != m_sTextures.end()){
            std::cout << "Texture " << name << " already loaded!" << std::endl;
            return;
        }

        std::shared_ptr<Texture> textureToLoad = std::make_shared<Texture>();
        AllocatedImage image = device.loadTexture(path);

        // setting up the texture
        textureToLoad->m_name = name;
        textureToLoad->m_allocatedImage = image;
        VkImageViewCreateInfo imageInfo = Initializers::createImageViewInfo(VK_FORMAT_R8G8B8A8_SRGB, textureToLoad->m_allocatedImage.m_image, VK_IMAGE_ASPECT_COLOR_BIT);
        vkCreateImageView(device.getDevice(), &imageInfo, nullptr, &textureToLoad->m_imageView);

        m_sTextures[name] = textureToLoad;
    }

    std::shared_ptr<Texture> AssetsManager::getTexture(const std::string &name) {
        if(m_sTextures.find(name) == m_sTextures.end()){
            std::cout << "Texture " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_sTextures[name];
    }
}