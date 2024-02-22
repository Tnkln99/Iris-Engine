#ifndef IRIS_NAVIGATIONAREA3D_HPP
#define IRIS_NAVIGATIONAREA3D_HPP

#include "../app/Objects.hpp"
#include "NavigationTile3D.hpp"

#include <map>

namespace iris::ai{
    class NavigationArea3D {
    public:
        void loadArea(glm::vec3 location, int gridSize, int height, int width, int depth,
                      std::vector<app::RenderObject>& renderObjects);

        std::map<int, NavigationTile3D> m_tiles; // Graph of tiles, obstacles marked
        std::map<int , std::vector<int>> m_navGraph;

        void aStarFindWay(glm::vec3 start, glm::vec3 target, std::vector<app::RenderObject>& renderObjects);
        void greedyBestFirstSearch(glm::vec3 start, glm::vec3 target, std::vector<app::RenderObject>& renderObjects);
    private:
        void generateGraph();
        glm::vec3 m_location;
        int m_gridSize;
        int m_height;
        int m_width;
        int m_depth;

        int worldPosToGridIndex(const glm::vec3& pos) {
            // Calculate steps from m_location in each dimension
            int stepX = static_cast<int>((pos.x - m_location.x) / m_gridSize);
            int stepY = static_cast<int>((pos.y - m_location.y) / m_gridSize);
            int stepZ = static_cast<int>((pos.z - m_location.z) / m_gridSize);

            // Calculate the linear index (id) based on steps
            int totalCellsPerLayer = (m_width / m_gridSize) * (m_height / m_gridSize);
            int id = stepZ * totalCellsPerLayer + stepY * (m_width / m_gridSize) + stepX;

            return id;
        }

        glm::vec3 gridIndexToWorldPos(int index) {
            int cellsPerX = m_width / m_gridSize;
            int cellsPerY = m_height / m_gridSize;
            int totalCellsPerLayer = cellsPerX * cellsPerY;

            int z = index / totalCellsPerLayer;
            int rem = index % totalCellsPerLayer;
            int y = rem / cellsPerX;
            int x = rem % cellsPerX;

            glm::vec3 cellMin = m_location + glm::vec3(x * m_gridSize, y * m_gridSize, z * m_gridSize) - (m_gridSize / 2.0f);
            glm::vec3 worldPos = (cellMin + glm::vec3(m_gridSize / 2.0f));

            return worldPos;
        }

        [[nodiscard]] double heuristic(int a, int b) const {
            return glm::distance(m_tiles.find(a)->second.m_location ,m_tiles.find(b)->second.m_location);
        }

        void drawPath(int start, int goal, std::unordered_map<int, int> cameFrom, std::vector<app::RenderObject>& renderObjects);

    };
}




#endif //IRIS_NAVIGATIONAREA3D_HPP
