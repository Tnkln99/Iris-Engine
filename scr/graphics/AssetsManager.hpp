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

        static void clear();
    private:
        static std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
    };
}




#endif //IRIS_ASSETSMANAGER_HPP
