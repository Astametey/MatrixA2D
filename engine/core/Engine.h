#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <vector>
#include <memory> // Для std::unique_ptr

#include "entities/Player.h"
#include "entities/Enemy.h"
#include "world/levelLoader.h"
#include "resources/inventory.h"
#include "world/LevelManager.h"

struct DrawableEntity {
    sf::Drawable* drawable;
    float yPosition;
};


class Engine {
public: 
    Engine();
    ~Engine();

    void run();

private:
    void handleEvents(float deltaTime);
    void update(float deltaTime);
    void render(float deltaTime);
    void initResources();
    void ClearAllEntities(); // Очистка при смене уровня
    std::vector<std::unique_ptr<Enemy>> enemies;
    LevelManager levelManager;
    void HandleLevelTransitions();
    void SpawnPlayerAtStart();

    float enemySpawnTimer = 0.0f;
    const float enemySpawnInterval = 1.0f; // Интервал спавна в секундах
    const int maxEnemies = 5; // Максимальное количество врагов
    std::mt19937 rng; // Генератор случайных чисел

    sf::RenderWindow m_window;
    

    Player player;//player obj
    Inventory inventory; // inventory

    sf::Vector2f cameraPosition;//cameras speed value
    sf::Vector2f cameraVelocity;
    const float cameraSmoothness = 8.0f; // Чем больше, тем плавнее

    // mouse pos in window
    sf::Vector2i mousePos;
    // mouse pos in view
    sf::Vector2f worldPos;

    //init Resources
    
    
};
