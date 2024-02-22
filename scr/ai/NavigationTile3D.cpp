#include "NavigationTile3D.hpp"

namespace iris::ai{
    NavigationTile3D::NavigationTile3D(glm::vec3 location, float size, int id,
                                       iris::ai::NavigationTile3D::TileType tileType) {
        m_location = location;
        m_size = size;
        m_id = id;
        m_tileType = tileType;
    }
}

