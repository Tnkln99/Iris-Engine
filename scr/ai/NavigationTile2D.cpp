#include "NavigationTile2D.hpp"
#include "../graphics/AssetsManager.hpp"

namespace iris::ai{
    void NavigationTile2D::changeType(iris::ai::NavigationTile2D::TileType type) {
        m_type = type;
        switch (type) {
            case WALKABLE:
                m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Walkable");
                break;
            case OBSTACLE:
                m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Obstacle");
                break;
            case TARGET:
                m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Target");
                break;
            case START:
                m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Man");
                break;
            case ROAD:
                m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Road");
                break;
            case EXPLORED:
                m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
                break;
            case NONE:
                break;
        }
    }
}