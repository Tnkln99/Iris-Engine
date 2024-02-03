#include "Actor.hpp"

namespace iris::core {

    Actor::Actor() {

    }

    Actor::~Actor() {

    }

    void Actor::addComponent(std::unique_ptr<Component> component) {
        m_components.push_back(std::move(component));
    }
}
