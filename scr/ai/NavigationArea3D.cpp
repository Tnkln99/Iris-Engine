#include <queue>
#include "NavigationArea3D.hpp"
#include "NavigationTile3D.hpp"
#include "NavigationTile2D.hpp"
#include "../graphics/AssetsManager.hpp"


namespace iris::ai{

    void NavigationArea3D::loadArea(glm::vec3 location, int gridSize, int height, int width, int depth,
                                    std::vector<app::RenderObject>& renderObjects) {
        m_location = location;
        m_gridSize = gridSize;
        m_height = height;
        m_width = width;
        m_depth = depth;
        int id = 0; // Unique identifier for each grid cell
        std::vector<int> obstacles;
        std::vector<app::RenderObject> tilesToAdd;
        for (int z = 0; z < depth; z += gridSize) {
            for (int y = 0; y < height; y += gridSize) {
                for (int x = 0; x < width; x += gridSize) {
                    glm::vec3 cellMin = location + glm::vec3(x, y, z) - (gridSize / 2.0f);
                    glm::vec3 cellMax = cellMin + glm::vec3(gridSize, gridSize, gridSize);

                    for(auto & renderobject : renderObjects){
                        if(renderobject.getBoundingBox().intersects(cellMin, cellMax)){
                            //app::RenderObject tileToAvoid{};
                            //tileToAvoid.m_transform.m_translation = (cellMin + cellMax) / 2.0f; // Center of the cell
                            //tileToAvoid.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Debug");
                            //tileToAvoid.m_transform.m_scale = glm::vec3(gridSize);
                            //tileToAvoid.setModel(graphics::AssetsManager::getModel("Cube"));
                            //tilesToAdd.push_back(tileToAvoid);
                            if (std::find(obstacles.begin(), obstacles.end(), id) == obstacles.end()) {
                                obstacles.push_back(id);
                            }
                        }

                        NavigationTile3D tile((cellMin + cellMax) / 2.0f, gridSize, id, NavigationTile3D::TileType::PASSABLE);
                        m_tiles.insert(std::pair<int, NavigationTile3D>(id, tile));
                    }

                    app::RenderObject tileToAvoid{};
                    tileToAvoid.m_transform.m_translation = (cellMin + cellMax) / 2.0f; // Center of the cell
                    tileToAvoid.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Debug");
                    tileToAvoid.m_transform.m_scale = glm::vec3(gridSize);
                    tileToAvoid.m_name = "grid";
                    tileToAvoid.m_bRender = false;
                    tileToAvoid.setModel(graphics::AssetsManager::getModel("Cube"));
                    tilesToAdd.push_back(tileToAvoid);
                    id++;
                }
            }
        }

        for(auto & tile : tilesToAdd){
            renderObjects.push_back(tile);
        }

        for (auto obsId : obstacles) {
            m_tiles.find(obsId)->second.m_tileType = NavigationTile3D::TileType::OBSTACLE;
        }

        generateGraph();
    }

    void NavigationArea3D::generateGraph() {
        std::vector<std::tuple<int, int, int>> directions = {
                // Cardinal directions
                std::make_tuple(1, 0, 0), std::make_tuple(-1, 0, 0),  // Right, Left
                std::make_tuple(0, 1, 0), std::make_tuple(0, -1, 0),  // Up, Down
                std::make_tuple(0, 0, 1), std::make_tuple(0, 0, -1),  // Forward, Backward

                // Edge diagonals on XY plane
                std::make_tuple(1, 1, 0), std::make_tuple(1, -1, 0),
                std::make_tuple(-1, 1, 0), std::make_tuple(-1, -1, 0),

                // Edge diagonals on XZ plane
                std::make_tuple(1, 0, 1), std::make_tuple(1, 0, -1),
                std::make_tuple(-1, 0, 1), std::make_tuple(-1, 0, -1),

                // Edge diagonals on YZ plane
                std::make_tuple(0, 1, 1), std::make_tuple(0, 1, -1),
                std::make_tuple(0, -1, 1), std::make_tuple(0, -1, -1),

                // Corner diagonals
                std::make_tuple(1, 1, 1), std::make_tuple(1, 1, -1),
                std::make_tuple(1, -1, 1), std::make_tuple(1, -1, -1),
                std::make_tuple(-1, 1, 1), std::make_tuple(-1, 1, -1),
                std::make_tuple(-1, -1, 1), std::make_tuple(-1, -1, -1)
        };

        // Assuming gridSize defines the size of each cell and cells are equally spaced
        int cellsX = m_width / m_gridSize; // Number of cells along the X dimension
        int cellsY = m_height / m_gridSize; // Number of cells along the Y dimension
        // No division for depth since z loop increments by gridSize already

        for (const auto& tile : m_tiles) {
            std::vector<int> neighbours;
            if (tile.second.m_tileType == NavigationTile3D::TileType::OBSTACLE) {
                continue;
            }

            int id = tile.first;
            int z = id / (cellsX * cellsY);
            int y = (id / cellsX) % cellsY;
            int x = id % cellsX;

            for (const auto& direction : directions) {
                int dx = std::get<0>(direction);
                int dy = std::get<1>(direction);
                int dz = std::get<2>(direction);

                int nx = x + dx;
                int ny = y + dy;
                int nz = z + dz;

                if (nx >= 0 && nx < cellsX && ny >= 0 && ny < cellsY && nz >= 0 && nz < m_depth / m_gridSize) {
                    int neighbourId = nz * (cellsX * cellsY) + ny * cellsX + nx;
                    auto findTile = m_tiles.find(neighbourId);
                    if (findTile != m_tiles.end() && findTile->second.m_tileType != NavigationTile3D::TileType::OBSTACLE) {
                        neighbours.push_back(neighbourId);
                    }
                }
            }

            m_navGraph[id] = neighbours;
        }
    }

    void NavigationArea3D::aStarFindWay(glm::vec3 start, glm::vec3 target, std::vector<app::RenderObject>& renderObjects) {
        int startIndex = worldPosToGridIndex(start);
        int goalIndex = worldPosToGridIndex(target);

        std::unordered_map<int, int> cameFrom{};
        std::unordered_map<int, double> costSoFar{};
        std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> frontier;
        frontier.emplace(0.0, startIndex);
        cameFrom[startIndex] = startIndex;
        costSoFar[startIndex] = 0.0;

        while (!frontier.empty()) {
            int current = frontier.top().second;
            frontier.pop();

            if (current == goalIndex) {
                break;
            }

            for (auto& next : m_navGraph[current]) {
                double newCost = costSoFar[current] + 1; // Assuming uniform cost for simplicity
                if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next]) {
                    costSoFar[next] = newCost;
                    double priority = newCost + heuristic(next, goalIndex); // Ensure heuristic returns double
                    frontier.emplace(priority, next);
                    cameFrom[next] = current;
                }
            }
        }

        drawPath(startIndex, goalIndex, cameFrom, renderObjects);
    }

    void NavigationArea3D::greedyBestFirstSearch(glm::vec3 start, glm::vec3 target,
                                                 std::vector<app::RenderObject> &renderObjects) {
        int startIndex = worldPosToGridIndex(start);
        int goalIndex = worldPosToGridIndex(target);

        std::unordered_map<int, int> cameFrom{};
        std::unordered_map<int, double> costSoFar{};
        std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> frontier;
        frontier.emplace(0.0, startIndex);
        cameFrom[startIndex] = startIndex;

        while (!frontier.empty()) {
            int current = frontier.top().second;
            frontier.pop();

            if (current == goalIndex) {
                break;
            }

            for (auto& next : m_navGraph[current]) {
                if (cameFrom.find(next) == cameFrom.end()) {
                    double priority =  heuristic(next, goalIndex); // Ensure heuristic returns double
                    frontier.emplace(priority, next);
                    cameFrom[next] = current;
                }
            }
        }

        drawPath(startIndex, goalIndex, cameFrom, renderObjects);
    }

    void
    NavigationArea3D::drawPath(int start, int goal, std::unordered_map<int, int> cameFrom, std::vector<app::RenderObject> &renderObjects) {
        if(cameFrom.find(goal) == cameFrom.end()){
            std::cout << "No path found" << std::endl;
            return;
        }

        std::vector<int> path;
        int current = goal;
        path.push_back(current);

        while(current != start) {
            current = cameFrom[current];
            path.push_back(current);
        }


        // The path vector now contains the indices from target to start, in reverse order
        // To print them in order from start to target, reverse the vector or iterate in reverse
        for(auto it = path.rbegin(); it != path.rend(); ++it) {
            if(*it == start){
                continue;
            }
            if(*it == goal){
                continue;
            }
            app::RenderObject cube{};
            cube.m_transform.m_translation = m_tiles.find(*it)->second.m_location;
            cube.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_StarTextured");
            cube.m_transform.m_scale = glm::vec3(0.5);
            cube.m_name = "path";
            cube.setModel(graphics::AssetsManager::getModel("Star"));
            cube.getBoundingBox().update(0, cube.modelMatrix());
            renderObjects.push_back(cube);
        }
    }
}