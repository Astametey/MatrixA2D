#pragma once
#include "levelLoader.h"
#include "entities/Enemy.h"
#include "resources/inventory.h"
#include <unordered_map>
#include <string>

class LevelManager {
private:
    struct LevelData {
        Level map;
        std::vector<std::unique_ptr<Enemy>> enemies;
        std::vector<std::unique_ptr<Item>> groundItems;
    };

    std::unordered_map<std::string, std::unique_ptr<Item>> itemTemplates;
    std::unordered_map<std::string, LevelData> levels;
    LevelData* currentLevelData = nullptr;

    void InitItemTemplates();
    void InitArmorTemplates();
    void InitConsumableTemplates();

public:
    std::unique_ptr<Item> CreateItemFromTemplate(const std::string& name,
        const sf::Vector2f& position);
    void LoadAllLevels();
    void SwitchLevel(const std::string& levelName);
    void LoadLevel(const std::string& levelName, const std::string& filePath);
    Level* GetCurrentLevel() { return currentLevelData ? &currentLevelData->map : nullptr; }
    std::vector<std::unique_ptr<Enemy>>& GetCurrentEnemies() { return currentLevelData->enemies; }
    std::vector<std::unique_ptr<Item>>& GetCurrentGroundItems() { return currentLevelData->groundItems; }
};