#ifndef IRIS_NAVIGATIONTILE2D_HPP
#define IRIS_NAVIGATIONTILE2D_HPP

#include "../app/Objects.hpp"

namespace iris::ai{
    class NavigationTile2D : public app::RenderObject{
    public:
        enum TileType{
            WALKABLE,
            BUSH,
            OBSTACLE,
            TARGET,
            START,
            ROAD,
            EXPLORED,
            EXPLORED_BUSH,
            ROAD_BUSH,
            NONE
        };
        int m_index;
        void changeType(TileType type);
        TileType getType() const { return m_type; }
    private:
        TileType m_type;
    };
}



#endif //IRIS_NAVIGATIONTILE2D_HPP
