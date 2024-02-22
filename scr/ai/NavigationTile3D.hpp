#ifndef IRIS_NAVIGATIONTILE3D_HPP
#define IRIS_NAVIGATIONTILE3D_HPP

#include "../app/Objects.hpp"
namespace iris::ai{
    class NavigationTile3D{
    public:
        enum TileType{
            PASSABLE,
            OBSTACLE
        }m_tileType = PASSABLE;

        NavigationTile3D(glm::vec3 location, float size, int id, TileType tileType);
        ~NavigationTile3D() = default;

        glm::vec3 m_location;
        float m_size;
        int m_id;
    private:
    };
}


#endif //IRIS_NAVIGATIONTILE3D_HPP
