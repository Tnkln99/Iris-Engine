#include "AssetsManager.hpp"

namespace iris::graphics{
    std::unordered_map<std::string, std::shared_ptr<Model>> AssetsManager::m_Models = std::unordered_map<std::string, std::shared_ptr<Model>>{};

    void AssetsManager::clear() {
        m_Models.clear();
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
}