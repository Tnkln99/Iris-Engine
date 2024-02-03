#ifndef IRIS_ACTOR_HPP
#define IRIS_ACTOR_HPP

#include "Component.hpp"

#include <vector>
#include <memory>

namespace iris::core{
    class Actor {
    public:
        Actor();
        ~Actor();

        bool m_bIsRenderable = false;

        void addComponent(std::unique_ptr<Component> component);
    private:
        std::vector<std::unique_ptr<Component>> m_components;
    };
}




#endif //IRIS_ACTOR_HPP
