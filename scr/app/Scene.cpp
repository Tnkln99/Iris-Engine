//
// Created by tanku on 21.02.2024.
//

#include "Scene.hpp"

namespace iris::app{

    Scene::Scene(graphics::ForwardRenderer &renderer, graphics::Window &window) : m_rRenderer(renderer), m_rWindow(window) {
        m_rRenderer.init();
        utils::Timer::init();

        m_sceneData.m_ambientLightColor = {1.f, 1.f, 1.f, .02f};
    }

    Scene::~Scene() {
        m_renderObjects.clear();
    }
}