#ifndef IRIS_NAVIGATIONAREA2D_HPP
#define IRIS_NAVIGATIONAREA2D_HPP

#include "NavigationTile2D.hpp"

namespace iris::ai{
    class NavigationArea2D {
    public:
    private:
        std::vector<NavigationTile2D> m_tiles{};
    };
}



#endif //IRIS_NAVIGATIONAREA2D_HPP
