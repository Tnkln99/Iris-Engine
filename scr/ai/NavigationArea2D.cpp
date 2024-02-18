#include "NavigationArea2D.hpp"
#include "../graphics/AssetsManager.hpp"

namespace iris::ai{

    void NavigationArea2D::loadArea() {
        int id = 0;
        float yOffset = -20.0f;
        for(int i = 0; i < 20; i++){
            float xOffset = -20.0f;
            for(int j = 0; j < 20; j++){
                NavigationTile2D square{};
                square.setModel(graphics::AssetsManager::getModel("Square"));
                square.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_FrameTextured");
                square.m_transform.m_translation = {xOffset, yOffset, 0.0f};
                square.m_transform.m_scale = {1.0f, 1.0f, 1.0f};
                square.m_index = id;
                square.changeType(NavigationTile2D::TileType::WALKABLE);
                m_tiles.insert(std::pair<int, NavigationTile2D> (id, square));
                xOffset += 2.0f;
                id++;
            }
            yOffset += 2.0f;
        }
    }

    void NavigationArea2D::BreadthFirstSearch() {

    }

    std::map<int, int> NavigationArea2D::generateNavigationGrid() {
        std::map<int, int> grid{};
        for(auto& tile : m_tiles){
            grid.insert(std::pair<int, int> (tile.first, 0));
        }
        return grid;
    }
}