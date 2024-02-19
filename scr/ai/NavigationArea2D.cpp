#include <queue>
#include "NavigationArea2D.hpp"
#include "../graphics/AssetsManager.hpp"

namespace iris::ai{

    void NavigationArea2D::loadArea() {
        int id = 0;
        float yOffset = -20.0f;
        for(int i = 0; i < m_height; i++){
            float xOffset = -20.0f;
            for(int j = 0; j < m_width; j++){
                NavigationTile2D square{};
                square.setModel(graphics::AssetsManager::getModel("Square"));
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

    void NavigationArea2D::breadthFirstSearch() {
        auto grid = generateNavigationGrid();
        std::unordered_map<int, int> cameFrom{};
        cameFrom.insert(std::pair<int, int> (m_startTileIndex, m_startTileIndex));
        std::queue<int> frontier{};
        frontier.push(m_startTileIndex);

        while(!frontier.empty()){
            int current = frontier.front();
            frontier.pop();
            if (current == m_targetTileIndex) {
                break;
            }
            for(auto& next : grid[current]){
                if(cameFrom.find(next) == cameFrom.end()){
                    frontier.push(next);
                    cameFrom.insert(std::pair<int, int> (next, current));
                }
            }
        }

        for(auto it = cameFrom.begin(); it != cameFrom.end(); it++){
            if(it->first == m_startTileIndex){
                continue;
            }
            if(it->first == m_targetTileIndex){
                continue;
            }
            m_tiles[it->first].changeType(NavigationTile2D::TileType::EXPLORED);
        }

        std::vector<int> path;
        int current = m_targetTileIndex;
        path.push_back(current);

        while(current != m_startTileIndex) {
            current = cameFrom[current];
            path.push_back(current);
        }

        // The path vector now contains the indices from target to start, in reverse order
        // To print them in order from start to target, reverse the vector or iterate in reverse
        for(auto it = path.rbegin(); it != path.rend(); ++it) {
            if(*it == m_startTileIndex){
                continue;
            }
            if(*it == m_targetTileIndex){
                continue;
            }
            m_tiles[*it].changeType(NavigationTile2D::TileType::ROAD);
        }
    }

    std::map<int, std::vector<int>> NavigationArea2D::generateNavigationGrid() {
        std::map<int, std::vector<int>> grid{};
        std::vector<std::pair<int, int>> directions = {
                std::pair<int, int> (0, 1),
                std::pair<int, int> (1, 0),
                std::pair<int, int> (0, -1),
                std::pair<int, int> (-1, 0)
        };
        std::vector<int> neighbours{};
        for(auto& tile : m_tiles){
            neighbours.clear();
            for(auto& direction : directions){
                if(tile.second.getType() == NavigationTile2D::TileType::OBSTACLE){
                    continue;
                }
                int index = tile.first;
                int x = index % m_height;
                int y = index / m_width;
                int xDir = direction.first;
                int yDir = direction.second;
                int xNeighbour = x + xDir;
                int yNeighbour = y + yDir;
                if(xNeighbour >= 0 && xNeighbour < m_width && yNeighbour >= 0 && yNeighbour < m_height){
                    int neighbourIndex = yNeighbour * 20 + xNeighbour;
                    if(m_tiles[neighbourIndex].getType() != NavigationTile2D::TileType::OBSTACLE){
                        neighbours.push_back(neighbourIndex);
                    }
                }
            }
            grid.insert(std::pair<int, std::vector<int>> (tile.first, neighbours));
        }
        return grid;
    }
}