#include "LevelManager.h"

void LevelManager::LoadAllLevels() {
    LoadLevel("forest", "resources/maps/forest_map.tmx");
    LoadLevel("forest_camp", "resources/maps/forest_map_camp.tmx");
    LoadLevel("town", "resources/maps/town_0_map.tmx");
    SwitchLevel("forest"); // Устанавливаем начальный уровень

    
    
}

void LevelManager::LoadLevel(const std::string& levelName, const std::string& filePath) {
    if (levels.count(levelName)) return;

    InitItemTemplates(); // Инициализируем шаблоны при первой загрузке

    auto& levelData = levels[levelName];
    if (!levelData.map.LoadFromFile(filePath, "resources/maps/forest_tile_set.png")) {
        std::cerr << "Failed to load level: " << levelName << std::endl;
        levels.erase(levelName);
        return;
    }

    for (const auto& object : levelData.map.getObjects()) { 
        if (object.type == "spawn") {
            auto enemy = std::make_unique<Enemy>(
                object.rect.left + object.rect.width / 2,
                object.rect.top + object.rect.height / 2,
                8, 16,
                levelData.map,  // Используем levelData.map вместо GetCurrentLevel()
                "resources/textures/enemies/cat.png"
            );
            enemy->Set_type_enemy(object.name);
            levelData.enemies.push_back(std::move(enemy));
        }
        if (object.type == "pickup") {
            sf::Vector2f position(
                object.rect.left + object.rect.width / 2 - 8,
                object.rect.top + object.rect.height / 2 - 8
            );

            // Создаем предмет по имени шаблона
            if (auto newItem = CreateItemFromTemplate(object.name, position)) {
                levelData.groundItems.push_back(std::move(newItem));
                std::cout << "Spawned item: " << object.name << " at "
                    << position.x << "," << position.y << std::endl;
            }
            else if (object.GetPropertyFloat("item_template")) {
                // Альтернативный вариант - использовать свойство item_template
                std::string templateName = object.GetPropertyString("item_template");
                if (auto newItem = CreateItemFromTemplate(templateName, position)) {
                    levelData.groundItems.push_back(std::move(newItem));
                    std::cout << "Spawned item from template: " << templateName << std::endl;
                }
            }
        }
    }
}


void LevelManager::SwitchLevel(const std::string& levelName) {
    auto it = levels.find(levelName);
    if (it != levels.end()) {
        currentLevelData = &it->second;
        std::cout << "Switched to level: " << levelName << std::endl;

        // Отладочный вывод
        std::cout << "Level contains:\n";
        std::cout << "- Enemies: " << currentLevelData->enemies.size() << "\n";
        std::cout << "- Items: " << currentLevelData->groundItems.size() << "\n";

        for (auto& item : currentLevelData->groundItems) {
            if (item->type == ItemType::WEAPON) {
                WeaponItem* weapon = dynamic_cast<WeaponItem*>(item.get());
                if (weapon) {
                    std::cout << "  Weapon: " << weapon->weapon.name
                        << " (DMG: " << weapon->weapon.damage << ")\n";
                }
            }
        }
    }
    else {
        std::cerr << "Level not found: " << levelName << std::endl;
    }
}

void LevelManager::InitItemTemplates() {
    // Оружие
    // Инициализация лука
    Weapon bowTemplate("resources/textures/items/weapon/bow.png");
    bowTemplate.name = "Железный лук";
    bowTemplate.item_description = "";
    bowTemplate.damage = 2.0f;
    bowTemplate.criticalChance = 0.08f;
    bowTemplate.setType(1);
    bowTemplate.setTextureRect(64, 0, 16, 16);

    auto ironBow = std::make_unique<WeaponItem>(bowTemplate, ItemType::WEAPON);
    itemTemplates["iron_bow"] = std::move(ironBow);

    // Инициализация меча
    Weapon swordTemplate("resources/textures/items/weapon/sword.png");
    swordTemplate.name = "Железный меч";
    swordTemplate.item_description = "";
    swordTemplate.damage = 5.0f;
    swordTemplate.criticalChance = 0.15f;
    swordTemplate.setType(0);
    swordTemplate.setTextureRect(32, 0, 16, 16);
    // Проверка загрузки текстуры
    if (swordTemplate.texture.getSize().x == 0) {
        std::cerr << "Failed to load sword texture!" << std::endl;
    }
    auto steelSword = std::make_unique<WeaponItem>(swordTemplate, ItemType::WEAPON);
    itemTemplates["steel_sword"] = std::move(steelSword);
    
    
    InitConsumableTemplates();
    InitArmorTemplates();
}

void LevelManager::InitArmorTemplates() {
    // head armor
    Armor ironHelmet("resources/textures/items/armor/helmet.png");
    ironHelmet.name = "Железный шлем";
    ironHelmet.item_description = "";
    ironHelmet.defense = 5.0f;
    ironHelmet.sprite.setTextureRect(sf::IntRect(64, 0, 16, 16)); // icon

    auto ironHelmetItem = std::make_unique<ArmorItem>(ironHelmet, ItemType::HELMET);
    itemTemplates["iron_helmet"] = std::move(ironHelmetItem);
    //-----------------//



    Armor headscarf("resources/textures/items/armor/headscarf.png");
    headscarf.name = "Головной платок";
    headscarf.item_description = "";
    headscarf.defense = 1.0f;
    headscarf.sprite.setTextureRect(sf::IntRect(64, 0, 16, 16)); // icon

    auto headscarfItem = std::make_unique<ArmorItem>(ironHelmet, ItemType::HELMET);
    itemTemplates["headscarf"] = std::move(headscarfItem);
    //-----------------//



    Armor dear_hat("resources/textures/items/armor/dear_hat.png");
    dear_hat.name = "Дорогая шляпа";
    dear_hat.item_description = "Дорогая шляпа, украшенная кружевным платком";
    dear_hat.defense = 2.0f;
    dear_hat.sprite.setTextureRect(sf::IntRect(64, 0, 16, 16)); // icon

    auto dearhatItem = std::make_unique<ArmorItem>(dear_hat, ItemType::HELMET);
    itemTemplates["dear_hat"] = std::move(dearhatItem);
    //-----------------//



    Armor tourist_hat("resources/textures/items/armor/tourist_hat.png");
    tourist_hat.name = "Шляпа туриста"; 
    tourist_hat.item_description = "";
    tourist_hat.defense = 3.0f;
    tourist_hat.sprite.setTextureRect(sf::IntRect(64, 0, 16, 16)); // icon

    auto touristhatItem = std::make_unique<ArmorItem>(ironHelmet, ItemType::HELMET);
    itemTemplates["tourist_hat"] = std::move(touristhatItem);
    //-----------------//


    // Armor
    Armor smallDress("resources/textures/items/armor/dress.png");
    smallDress.name = "Маленькое платье";
    smallDress.item_description = "";
    smallDress.defense = 5.0f;
    smallDress.sprite.setTextureRect(sf::IntRect(64, 0, 16, 16));
    
    auto smallDressItem = std::make_unique<ArmorItem>(smallDress, ItemType::BODY);
    smallDressItem->sprite.setTextureRect(smallDress.sprite.getTextureRect());
    itemTemplates["small_dress"] = std::move(smallDressItem);
    

    // 
    Armor ironJacket("resources/textures/items/armor/chest.png");
    ironJacket.name = "Железная куртка";
    ironJacket.item_description = "";
    ironJacket.defense = 10.0f;
    ironJacket.sprite.setTextureRect(sf::IntRect(64, 0, 16, 16));
    
    auto ironJacketItem = std::make_unique<ArmorItem>(ironJacket, ItemType::BODY);
    ironJacketItem->sprite.setTextureRect(ironJacket.sprite.getTextureRect());
    itemTemplates["iron_jacket"] = std::move(ironJacketItem);
    
    // legs
    Armor ironLeggings("resources/textures/items/armor/leggings.png");
    ironLeggings.name = "Железные поножи";
    ironLeggings.item_description = "";
    ironLeggings.defense = 7.0f;
    ironLeggings.sprite.setTextureRect(sf::IntRect(64, 0, 16, 16));

    auto ironLeggingsItem = std::make_unique<ArmorItem>(ironLeggings, ItemType::LEGS);
    ironLeggingsItem->sprite.setTextureRect(ironLeggings.sprite.getTextureRect());
    itemTemplates["iron_leggings"] = std::move(ironLeggingsItem);

    // Розовая юбка
    Armor pinkSkirt("resources/textures/items/armor/skirts.png");
    pinkSkirt.name = "Розовая юбка";
    pinkSkirt.item_description = "";
    pinkSkirt.defense = 4.0f;
    pinkSkirt.sprite.setTextureRect(sf::IntRect(64, 0, 16, 16));
    
    auto pinkSkirtItem = std::make_unique<ArmorItem>(pinkSkirt, ItemType::LEGS);
    pinkSkirtItem->sprite.setTextureRect(pinkSkirt.sprite.getTextureRect());
    itemTemplates["pink_skirt"] = std::move(pinkSkirtItem);
}

void LevelManager::InitConsumableTemplates()
{
    // Расходники
    auto healthPotion = std::make_unique<ConsumableItem>("Зелье здоровья", 25.0f, true);
    healthPotion->texture.loadFromFile("resources/textures/items/consumables/health_potion.png");
    healthPotion->sprite.setTexture(healthPotion->texture);
    healthPotion->sprite.setTextureRect(sf::IntRect(0, 0, 16, 16));
    itemTemplates["health_potion"] = std::move(healthPotion);

    auto bloodMushroom = std::make_unique<ConsumableItem>("Кровяной гриб", 5.0f, true);
    bloodMushroom->texture.loadFromFile("resources/textures/items/consumables/mushroom.png");
    bloodMushroom->sprite.setTexture(bloodMushroom->texture);
    bloodMushroom->isPlant = true;
    bloodMushroom->sprite.setTextureRect(sf::IntRect(0, 0, 16, 16));
    itemTemplates["blood_mushroom"] = std::move(bloodMushroom);
}

std::unique_ptr<Item> LevelManager::CreateItemFromTemplate(const std::string& name,
    const sf::Vector2f& position) {
    auto it = itemTemplates.find(name);
    if (it != itemTemplates.end()) {
        auto newItem = it->second->Clone();
        if (!newItem) {
            std::cerr << "Failed to clone item: " << name << std::endl;
            return nullptr;
        }
        newItem->setPosition(position);
        return newItem;
    }
    std::cerr << "Item template not found: " << name << std::endl;
    return nullptr;
}
