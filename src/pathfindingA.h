#ifndef PATHFIND_H
#define PATHFIND_H

#include <queue>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <functional> // Для std::greater
#include <memory>    // Для std::unique_ptr

#include "level.h"

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

// Оптимизация: предварительное вычисление эвристики
inline float calculateHeuristic(int x1, int y1, int x2, int y2)
{
    return std::abs(x1 - x2) + std::abs(y1 - y2); // Манхэттенское расстояние
}

// Оптимизация: кеширование проверок проходимости
bool isTileWalkable(int x, int y, const Level& level)
{
    static std::unordered_map<int, bool> walkableCache;
    int key = x + y * level.width;

    if (walkableCache.find(key) != walkableCache.end()) {
        return walkableCache[key];
    }

    if (x < 0 || x >= level.width || y < 0 || y >= level.height) {
        walkableCache[key] = false;
        return false;
    }

    if (level.layers.empty()) {
        walkableCache[key] = false;
        return false;
    }

    for (const auto& object : level.getObjects()) {
        if (object.type == "solid" &&
            x * level.tileWidth >= object.rect.left &&
            x * level.tileWidth < object.rect.left + object.rect.width &&
            y * level.tileHeight >= object.rect.top &&
            y * level.tileHeight < object.rect.top + object.rect.height) {
            walkableCache[key] = false;
            return false;
        }
    }

    walkableCache[key] = true;
    return true;
}

std::vector<sf::Vector2f> findPath(sf::Vector2f startPosition, sf::Vector2f targetPosition, const Level& level)
{
    // 1. Преобразование координат
    int startX = static_cast<int>(startPosition.x / level.tileWidth);
    int startY = static_cast<int>(startPosition.y / level.tileHeight);
    int targetX = static_cast<int>(targetPosition.x / level.tileWidth);
    int targetY = static_cast<int>(targetPosition.y / level.tileHeight);

    // 2. Быстрая проверка на тривиальные случаи
    if (startX == targetX && startY == targetY) {
        return { targetPosition };
    }

    if (!isTileWalkable(startX, startY, level) || !isTileWalkable(targetX, targetY, level)) {
        return {};
    }

    // 3. Оптимизированные структуры данных
    struct NodeCompare {
        bool operator()(const Node* a, const Node* b) const {
            return a->getTotalCost() > b->getTotalCost();
        }
    };

    std::priority_queue<Node*, std::vector<Node*>, NodeCompare> openSet;
    std::unordered_map<int, std::unique_ptr<Node>> allNodes;

    // 4. Использование указателей для избежания копирования
    auto startNode = std::make_unique<Node>(startX, startY);
    startNode->heuristic = calculateHeuristic(startX, startY, targetX, targetY);
    allNodes[startX + startY * level.width] = std::move(startNode);
    openSet.push(allNodes[startX + startY * level.width].get());

    // 5. Оптимизация направления поиска
    constexpr int dx[] = { 0, 1, 0, -1 }; // По часовой стрелке
    constexpr int dy[] = { 1, 0, -1, 0 };

    while (!openSet.empty())
    {
        Node* current = openSet.top();
        openSet.pop();

        if (current->x == targetX && current->y == targetY)
        {
            std::vector<sf::Vector2f> path;
            Node* node = current;
            while (node != nullptr)
            {
                path.emplace_back(
                    node->x * level.tileWidth + level.tileWidth / 2,
                    node->y * level.tileHeight + level.tileHeight / 2
                );
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (int i = 0; i < 4; ++i)
        {
            int neighborX = current->x + dx[i];
            int neighborY = current->y + dy[i];
            int neighborId = neighborX + neighborY * level.width;

            if (!isTileWalkable(neighborX, neighborY, level)) {
                continue;
            }

            float newCost = current->cost + 1.0f;

            auto it = allNodes.find(neighborId);
            if (it == allNodes.end())
            {
                auto neighborNode = std::make_unique<Node>(neighborX, neighborY);
                neighborNode->cost = newCost;
                neighborNode->heuristic = calculateHeuristic(neighborX, neighborY, targetX, targetY);
                neighborNode->parent = current;
                openSet.push(neighborNode.get());
                allNodes[neighborId] = std::move(neighborNode);
            }
            else if (newCost < it->second->cost)
            {
                it->second->cost = newCost;
                it->second->parent = current;
                openSet.push(it->second.get()); // Дублирование - не идеально, но работает
            }
        }
    }

    return {};
}

#endif