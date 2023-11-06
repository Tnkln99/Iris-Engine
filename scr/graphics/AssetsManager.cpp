#include "AssetsManager.hpp"
#include "Initializers.hpp"


namespace iris::graphics{
    std::unordered_map<std::string, std::shared_ptr<Model>> AssetsManager::m_Models = std::unordered_map<std::string, std::shared_ptr<Model>>{};
    std::unordered_map<std::string, std::shared_ptr<Material>> AssetsManager::m_Materials = std::unordered_map<std::string, std::shared_ptr<Material>>{};
    std::unordered_map<std::string, std::shared_ptr<Texture>> AssetsManager::m_Textures = std::unordered_map<std::string, std::shared_ptr<Texture>>{};

    void AssetsManager::clear(Device& device) {
        m_Models.clear();
        for(auto & material : m_Materials){
            vkDestroyPipelineLayout(device.getDevice(), material.second->getPipeLineLayout(), nullptr);
        }

        m_Materials.clear();

        for(auto & texture : m_Textures){
            vkDestroyImageView(device.getDevice(), texture.second->imageView, nullptr);
            vkDestroyImage(device.getDevice(), texture.second->image.image, nullptr);
        }
        m_Textures.clear();
    }

    void AssetsManager::loadModel(Device& device, const std::string& name, const std::string& path){
        if(m_Models.find(name) != m_Models.end()){
            std::cout << "Model " << name << " already loaded!" << std::endl;
            return;
        }

        m_Models[name] = Model::createModelFromFile(device, path);
    }

    std::shared_ptr<Model> AssetsManager::getModel(const std::string &name) {
        if(m_Models.find(name) == m_Models.end()){
            std::cout << "Model " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_Models[name];
    }

    void AssetsManager::loadMaterial(Device& device, const std::string &name, const std::shared_ptr<Pipeline>& pipeline, VkPipelineLayout layout,
                                     VkDescriptorSet texture) {
        if(m_Materials.find(name) != m_Materials.end()){
            std::cout << "Material " << name << " already loaded!" << std::endl;
            return;
        }
         m_Materials[name] = std::make_shared<Material>(device, name, pipeline, layout, texture);
    }

    std::shared_ptr<Material> AssetsManager::getMaterial(const std::string &name) {
        if(m_Materials.find(name) == m_Materials.end()){
            std::cout << "Material " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_Materials[name];
    }

    void AssetsManager::loadTexture(Device &device, const std::string &name, const std::string &path) {
        if(m_Textures.find(name) != m_Textures.end()){
            std::cout << "Texture " << name << " already loaded!" << std::endl;
            return;
        }

        std::shared_ptr<Texture> textureToLoad = std::make_shared<Texture>();
        AllocatedImage image = device.loadTexture(path);

        textureToLoad->name = name;
        textureToLoad->image = image;

        VkImageViewCreateInfo imageInfo = Initializers::createImageViewInfo(VK_FORMAT_R8G8B8A8_SRGB, textureToLoad->image.image, VK_IMAGE_ASPECT_COLOR_BIT);
        vkCreateImageView(device.getDevice(), &imageInfo, nullptr, &textureToLoad->imageView);

        m_Textures[name] = textureToLoad;
    }

    std::shared_ptr<Texture> AssetsManager::getTexture(const std::string &name) {
        if(m_Textures.find(name) == m_Textures.end()){
            std::cout << "Texture " << name << " does not exists" << std::endl;
            return nullptr;
        }

        return m_Textures[name];
    }
}