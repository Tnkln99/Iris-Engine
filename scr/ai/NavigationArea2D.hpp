#ifndef IRIS_NAVIGATIONAREA2D_HPP
#define IRIS_NAVIGATIONAREA2D_HPP

#include "NavigationTile2D.hpp"
#include <map>
#include <queue>

namespace iris::ai{
    class NavigationArea2D {
    public:
        void loadArea();
        std::map<int, NavigationTile2D> m_tiles{};
        int m_startTileIndex = -1;
        int m_targetTileIndex = -1;

        void breadthFirstSearch();
        void dijkstra(int bushWeight);
        void greedyBestFirstSearch();
        void aStar(int bushWeight);


        void resetPath();
        int m_height = 20;
        int m_width = 20;
    private:
        std::map<int, std::vector<int>> generateNavigationGrid();
        [[nodiscard]] int cost(int to, int bushCost) const {
            return m_tiles.find(to)->second.getType() == NavigationTile2D::TileType::BUSH ? bushCost : 1;
        }

        void drawExploredAndPath(std::unordered_map<int, int> cameFrom);
        float heuristic(int a, int b) const;
    };
}



#endif //IRIS_NAVIGATIONAREA2D_HPP
