#ifndef IRIS_NAVIGATIONAREA2D_HPP
#define IRIS_NAVIGATIONAREA2D_HPP

#include <map>
#include "NavigationTile2D.hpp"

namespace iris::ai{
    class NavigationArea2D {
    public:
        void loadArea();
        std::map<int, NavigationTile2D> m_tiles{};
        int m_startTileIndex = -1;
        int m_targetTileIndex = -1;

        void breadthFirstSearch();
        int m_height = 20;
        int m_width = 20;
        std::map<int, std::vector<int>> generateNavigationGrid();
    private:
    };
}



#endif //IRIS_NAVIGATIONAREA2D_HPP
