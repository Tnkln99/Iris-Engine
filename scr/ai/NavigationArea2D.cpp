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
            bool found = false;
            exploreBreadthFirstSearch(grid, cameFrom, frontier);
        }

        drawExploredAndPath(cameFrom);
    }

    void
    NavigationArea2D::exploreBreadthFirstSearch(std::map<int, std::vector<int>>& grid,
                                                std::unordered_map<int, int> &cameFrom,
                                                std::queue<int> &frontier, bool &found) {
        int current = frontier.front();
        frontier.pop();
        if (current == m_targetTileIndex) {
            found = true;
            return;
        }
        for(auto& next : grid[current]){
            if(cameFrom.find(next) == cameFrom.end()){
                frontier.push(next);
                cameFrom.insert(std::pair<int, int> (next, current));
            }
        }
    }


    void NavigationArea2D::dijkstra(int bushWeight) {
        auto grid = generateNavigationGrid();
        std::unordered_map<int, int> cameFrom{};
        std::unordered_map<int, double> costSoFar{};
        cameFrom[m_startTileIndex] = m_startTileIndex;
        costSoFar[m_startTileIndex] = 0;

        typedef std::pair<double, int> costToIndex;
        std::priority_queue<costToIndex, std::vector<costToIndex>, std::greater<>> frontier;
        frontier.emplace(0, m_startTileIndex);

        while (!frontier.empty()) {
            int current = frontier.top().second;
            frontier.pop();

            if (current == m_targetTileIndex) {
                break;
            }

            for (auto& neighbor : grid[current]) {
                double newCost = costSoFar[current] + cost(neighbor, bushWeight);
                if (costSoFar.find(neighbor) == costSoFar.end() || newCost < costSoFar[neighbor]) {
                    costSoFar[neighbor] = newCost;
                    cameFrom[neighbor] = current;
                    frontier.emplace(newCost, neighbor);
                }
            }
        }


        drawExploredAndPath(cameFrom);
    }

    void NavigationArea2D::greedyBestFirstSearch() {
        auto grid = generateNavigationGrid();
        std::unordered_map<int, int> cameFrom{};
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> frontier;
        frontier.emplace(0, m_startTileIndex);
        cameFrom[m_startTileIndex] = m_startTileIndex;

        while (!frontier.empty()) {
            int current = frontier.top().second;
            frontier.pop();

            if (current == m_targetTileIndex) {
                break;
            }

            for (auto& next : grid[current]) {
                if (cameFrom.find(next) == cameFrom.end()) {
                    int priority = heuristic(next, m_targetTileIndex);
                    frontier.emplace(priority, next);
                    cameFrom[next] = current;
                }
            }
        }

        drawExploredAndPath(cameFrom);
    }

    void NavigationArea2D::aStar(int bushWeight) {
        auto grid = generateNavigationGrid();
        std::unordered_map<int, int> cameFrom{};
        std::unordered_map<int, double> costSoFar{};
        std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> frontier;
        frontier.emplace(0, m_startTileIndex);
        cameFrom[m_startTileIndex] = m_startTileIndex;
        costSoFar[m_startTileIndex] = 0;

        while (!frontier.empty()) {
            int current = frontier.top().second;
            frontier.pop();

            if (current == m_targetTileIndex) {
                break;
            }

            for (auto& next : grid[current]) {
                double newCost = costSoFar[current] + cost(next, bushWeight);
                if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next]) {
                    costSoFar[next] = newCost;
                    int priority = newCost + heuristic(next, m_targetTileIndex);
                    frontier.emplace(priority, next);
                    cameFrom[next] = current;
                }
            }
        }

        drawExploredAndPath(cameFrom);
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

    void NavigationArea2D::resetPath() {
        for(auto & tile : m_tiles){
            if(tile.second.getType() == ai::NavigationTile2D::TileType::ROAD || tile.second.getType() == ai::NavigationTile2D::TileType::EXPLORED){
                tile.second.changeType(ai::NavigationTile2D::TileType::WALKABLE);
            }
            else if(tile.second.getType() == ai::NavigationTile2D::TileType::EXPLORED_BUSH || tile.second.getType() == ai::NavigationTile2D::TileType::ROAD_BUSH){
                tile.second.changeType(ai::NavigationTile2D::TileType::BUSH);
            }
        }
    }

    void NavigationArea2D::drawExploredAndPath(std::unordered_map<int, int> cameFrom) {
        for(auto it = cameFrom.begin(); it != cameFrom.end(); it++){
            if(it->first == m_startTileIndex){
                continue;
            }
            if(it->first == m_targetTileIndex){
                continue;
            }
            if(m_tiles[it->first].getType() == NavigationTile2D::TileType::BUSH){
                m_tiles[it->first].changeType(NavigationTile2D::TileType::EXPLORED_BUSH);
            }
            else{
                m_tiles[it->first].changeType(NavigationTile2D::TileType::EXPLORED);
            }
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
            if(m_tiles[*it].getType() == NavigationTile2D::TileType::EXPLORED_BUSH){
                m_tiles[*it].changeType(NavigationTile2D::TileType::ROAD_BUSH);
            }
            else{
                m_tiles[*it].changeType(NavigationTile2D::TileType::ROAD);
            }

        }
    }

    float NavigationArea2D::heuristic(int a, int b) const {
        int x1 = a % m_width;
        int y1 = a / m_height;
        int x2 = b % m_width;
        int y2 = b / m_height;
        return std::abs(x1 - x2) + std::abs(y1 - y2);
    }
}