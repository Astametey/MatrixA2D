#ifndef PATHFIND_H
#define PATHFIND_H

#include <queue>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <functional> // Для std::greater
#include <memory>    // Для std::unique_ptr

#include "levelLoader.h"

struct Node
{
    int x;
    int y;
    float cost;
    float heuristic;
    Node* parent;

    Node(int x, int y) : x(x), y(y), cost(0.0f), heuristic(0.0f), parent(nullptr) {}

    float getTotalCost() const { return cost + heuristic; }
};

// Объявления функций (без реализации)
float calculateHeuristic(int x1, int y1, int x2, int y2);
bool isTileWalkable(int x, int y, const Level& level);
std::vector<sf::Vector2f> findPath(sf::Vector2f startPosition, sf::Vector2f targetPosition, const Level& level);

#endif
