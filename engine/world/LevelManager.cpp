#include "LevelManager.h"

void LevelManager::LoadAllLevels() {
    LoadLevel("forest", "resources/maps/forest_map.tmx");
    LoadLevel("forest_camp", "resources/maps/forest_map_camp.tmx");
    SwitchLevel("forest"); // Устанавливаем начальный уровень
}

void LevelManager::LoadLevel(const std::string& levelName, const std::string& filePath) {
    if (levels.count(levelName)) return;

        auto& levelData = levels[levelName];
        if (!levelData.map.LoadFromFile(filePath, "resources/maps/forest_tile_set.png")) {
            std::cerr << "Failed to load level: " << levelName << std::endl;
                levels.erase(levelName);
                return;
        }

    // Загрузка врагов и объектов для уровня
        for (const auto& object : levelData.map.getObjects()) {
            if (object.type == "spawn") {
                auto enemy = std::make_unique<Enemy>(
                    object.rect.left + object.rect.width / 2,
                    object.rect.top + object.rect.height / 2,
                    16, 16,
                    levelData.map,  // Используем levelData.map вместо GetCurrentLevel()
                    "resources/textures/enemies/cat.png"
                );
                enemy->Set_type_enemy(object.name);
                levelData.enemies.push_back(std::move(enemy));
            }
            else if (object.type == "pickup") {
                if (object.name == "health_potion") {
                    auto healthPotion = std::make_unique<ConsumableItem>("Health Potion", 25.0f, true);
                    if (!healthPotion->texture.loadFromFile("resources/textures/items/consumables/health_potion.png")) {
                        std::cerr << "Failed to load health potion texture!" << std::endl;
                    }
                    healthPotion->sprite.setTexture(healthPotion->texture);
                    healthPotion->sprite.setTextureRect(sf::IntRect(0, 0, 16, 16));
                    levelData.groundItems.push_back(std::move(healthPotion));
                }
                else if (object.name == "mushroom") {
                    static sf::Texture mushroomTexture; // Статическая текстура
                    static bool isLoaded = false;

                    if (!isLoaded) {
                        if (!mushroomTexture.loadFromFile("resources/textures/items/consumables/mushroom.png")) {
                            std::cerr << "Failed to load mushroom texture!" << std::endl;
                            continue;
                        }
                        isLoaded = true;
                    }

                    auto mushroom = std::make_unique<ConsumableItem>("Magic Mushroom", 10.0f, true);
                    mushroom->texture = mushroomTexture;
                    mushroom->sprite.setTexture(mushroom->texture);
                    mushroom->sprite.setTextureRect(sf::IntRect(0, 0, 16, 16));
                    mushroom->setPosition(sf::Vector2f(object.rect.left, object.rect.top));
                    levelData.groundItems.push_back(std::move(mushroom));
                }
            }
        }
}

void LevelManager::SwitchLevel(const std::string& levelName) {
    auto it = levels.find(levelName);
    if (it != levels.end()) {
        currentLevelData = &it->second;
        std::cout << "Switched to level: " << levelName << std::endl;
    }
    else {
        std::cerr << "Level not found: " << levelName << std::endl;
        // Можно добавить загрузку уровня "на лету" при необходимости
        // LoadLevel(levelName, "resources/maps/" + levelName + ".tmx");
        // currentLevel = &levels.at(levelName);
    }
}

