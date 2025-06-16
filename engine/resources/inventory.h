#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "entities/Player.h"

enum class ItemType {
    HELMET,
    BODY,
    LEGS,
    SHOES,
    WEAPON,
    CONSUMABLE

};

class Item {
public:
    ItemType type;
    std::string name;
    sf::Sprite sprite;
    sf::Vector2f originalPosition;
    sf::Vector2f dragOffset;  // Добавляем смещение для перетаскивания
    bool isDragging = false;
    bool isOnGround = false;
    bool isEquipped = false;
    bool isStackable = false;
    bool canBeHit = true; // Новый флаг вместо wasHit
    float hitCooldown = 0.3f; // Защита от повторных попаданий
    float currentHitCooldown = 0.0f;

    bool isPlant = false; // Является ли предмет растением (грибом)
    int stackSize = 1;
    int maxStackSize = 1;

    sf::IntRect getRect;

    virtual ~Item() = default;

    // Метод для проверки возможности добавления в стек
    bool canStackWith(const Item& other) const {
        return isStackable &&
            type == other.type &&
            name == other.name &&
            stackSize < maxStackSize;
    }

    void setPosition(const sf::Vector2f& position) {
        sprite.setPosition(position);
        if (!isDragging) {
            originalPosition = position;
        }
    }

    void startDragging(const sf::Vector2f& mousePos) {
        isDragging = true;
        dragOffset = sprite.getPosition() - mousePos;
    }

    // метод для обновления позиции при перетаскивании
    void updateDragging(const sf::Vector2f& mousePos) {
        if (isDragging) {
            sprite.setPosition(mousePos + dragOffset);
        }
    }

    void update(float deltaTime)
    {
        if (!canBeHit) {
            currentHitCooldown -= deltaTime;
            if (currentHitCooldown <= 0) {
                canBeHit = true;
            }
        }
    }

    void scaleToSize(float width, float height) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        if (bounds.width > 0 && bounds.height > 0) {  // Защита от деления на 0
            sprite.setScale(width / bounds.width, height / bounds.height);
        }
    }
    virtual void equip(Player& player) = 0;
    virtual void unequip(Player& player) = 0;

    virtual std::unique_ptr<Item> Clone() const = 0;
};

class ConsumableItem : public Item {
public:
    float healthRestore;
    bool isPotion;
    sf::Texture texture;

    ConsumableItem(const std::string& name, float restoreAmount, bool potion = false) {
        type = ItemType::CONSUMABLE;
        this->name = name;
        healthRestore = restoreAmount;
        isPotion = potion;
        isStackable = true;
        maxStackSize = 99;
        texture.setSmooth(false);
    }

    void equip(Player& player) override {
        use(player);
    }

    void unequip(Player& player) override {
        // Ничего не делаем
    }

    void use(Player& player) {
        //player.heal(healthRestore);
        std::cout << "Used " << name << ", restored " << healthRestore << " health" << std::endl;
    }

    std::unique_ptr<Item> Clone() const override {
        auto clone = std::make_unique<ConsumableItem>(name, healthRestore, isPotion);
        clone->texture = this->texture;
        clone->sprite = this->sprite;
        clone->sprite.setTexture(clone->texture);
        clone->isStackable = this->isStackable;
        clone->maxStackSize = this->maxStackSize;
        clone->stackSize = this->stackSize;
        clone->isPlant = this->isPlant;
        return clone;
    }
};

class ArmorItem : public Item {
public:
    Armor armor;
    sf::Texture texture;

    ArmorItem(const Armor& armor, ItemType item_tipe) : armor(armor) {
        type = item_tipe;
        name = armor.name;

        if (!texture.loadFromFile(armor.texturePath)) {
            std::cerr << "Failed to load weapon texture: " << armor.texturePath << std::endl;
        }
        else {
            texture.setSmooth(false);
            sprite.setTextureRect(armor.sprite.getTextureRect());
            getRect = armor.sprite.getTextureRect();
            sprite.setTexture(texture);
        }
        
    }

    void equip(Player& player) override {
        isEquipped = true;
        if (type == ItemType::HELMET)
        {
            player.setArmorHead(armor);
        }
        if (type == ItemType::BODY)
        {
            player.setArmorBody(armor);
        }
        if (type == ItemType::LEGS)
        {
            player.setArmorLegs(armor);
        }
        if (type == ItemType::SHOES)
        {
            player.setArmorShoes(armor);
        }
    }

    void unequip(Player& player) override {
        isEquipped = false;
        if (type == ItemType::HELMET)
        {
            player.unqArmorHead();
        }
        if (type == ItemType::BODY)
        {
            player.unqArmorBody();
        }
        if (type == ItemType::LEGS)
        {
            player.unqArmorLegs();
        }
        if (type == ItemType::SHOES)
        {
            player.unqArmorShoes();
        }
    }

    std::unique_ptr<Item> Clone() const override {
        auto clone = std::make_unique<ArmorItem>(armor, type);

        if (!clone->texture.loadFromFile(armor.texturePath)) {
            std::cerr << "Failed to reload texture in clone: " << armor.texturePath << std::endl;
        }
        clone->sprite.setTexture(clone->texture);
        clone->sprite.setTextureRect(this->sprite.getTextureRect());
        clone->sprite.setPosition(this->sprite.getPosition());
        clone->sprite.setScale(this->sprite.getScale());
        return clone;
    }


};

class WeaponItem : public Item {
public:
    Weapon weapon;
    sf::Texture texture;

    WeaponItem(const Weapon& weaponTemplate, ItemType item_type)
        : weapon(weaponTemplate) {
        type = item_type;
        name = weapon.name;

        if (!texture.loadFromFile(weapon.texturePath)) {
            std::cerr << "Failed to load weapon texture: " << weapon.texturePath << std::endl;
        }
        else {
            sprite.setTexture(texture);
            sprite.setTextureRect(weapon.textureRect);
            getRect = weapon.textureRect;
        }
    }

    void equip(Player& player) override {
        isEquipped = true;
        player.setWeapon(weapon);
        std::cout << "Equipped weapon: " << name << std::endl;
    }

    void unequip(Player& player) override {
        isEquipped = false;
        player.unqWeapon();
        std::cout << "Unequipped weapon: " << name << std::endl;
    }

    std::unique_ptr<Item> Clone() const override {
        auto clone = std::make_unique<WeaponItem>(weapon, type);
        // Перезагружаем текстуру для клона
        if (!clone->texture.loadFromFile(weapon.texturePath)) {
            std::cerr << "Failed to reload texture in clone: " << weapon.texturePath << std::endl;
        }
        clone->sprite.setTexture(clone->texture);
        clone->sprite.setTextureRect(this->sprite.getTextureRect());
        clone->sprite.setPosition(this->sprite.getPosition());
        clone->sprite.setScale(this->sprite.getScale());
        clone->weapon.attackSpeed = this->weapon.attackSpeed;
        
        return clone;
    }
};

class Inventory {
public:
    Inventory(sf::RenderWindow& window, Player& player);
    bool addItem(std::unique_ptr<Item> newItem);
    bool isSlotOccupied(const sf::Vector2f& position) const;
    void update(float dt, sf::View& gui_view);
    void render();
    void renderUI();
    void handleEvent(const sf::Event& event, sf::View& gui_view);
    bool handleItemPickup(const sf::Event& event);
    void handleItemDrop(const sf::Event& event);
    bool tryPlaceInQuickSlot(const sf::Vector2f& mousePos);
    bool tryEquipItem(const sf::Vector2f& mousePos);
    bool tryPlaceInRegularSlot(const sf::Vector2f& mousePos);
    bool tryMergeStacks(Item& source, Item& target);
    void returnItemToOriginalPosition();
    bool addItemToFirstFreeSlot(std::unique_ptr<Item> item);
    bool canPlaceInQuickSlot(ItemType type) const;
    bool isEquipmentSlotPosition(const sf::Vector2f& pos) const;
    bool isQuickSlotPosition(const sf::Vector2f& pos) const;
    void updateEquipmentAfterQuickSlotChange(int slotIndex);
    void handleItemDrag(const sf::Event& event);
    void handleMouseWheelScroll(const sf::Event& event);
    void handleKeyPress(const sf::Event& event);
    bool handleShiftClick(const sf::Vector2f& mousePos);
    bool isArmorType(ItemType type) const;
    void selectQuickSlot(int slotIndex);
    void useActiveQuickSlotItem();
    void toggleVisibility();
    void tryPickupItem(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Item>>& groundItems, float deltaTime);
    bool isVisible() const;
    bool moveArmorToSlot(std::unique_ptr<Item> item, ItemType armorType);

    int activeQuickSlot = -1; // Индекс активного слота быстрого доступа
private:
    sf::RenderWindow& window;
    Player& player;

    std::vector<std::unique_ptr<Item>> items;
    std::unique_ptr<Item> equippedHelmet;
    std::unique_ptr<Item> equippedBody;
    std::unique_ptr<Item> equippedLegs;
    std::unique_ptr<Item> equippedShoes;

    sf::RectangleShape helmetSlot;
    sf::RectangleShape bodySlot;
    sf::RectangleShape legsSlot;
    sf::RectangleShape shoesSlot;
    sf::RectangleShape inventoryBackground;

    // Добавляем поля для хранения типов слотов
    ItemType helmetSlotType = ItemType::HELMET;
    ItemType bodySlotType = ItemType::BODY;
    ItemType legsSlotType = ItemType::LEGS;
    ItemType shoesSlotType = ItemType::SHOES;
    ItemType weaponSlotType = ItemType::WEAPON;

    std::unique_ptr<Item> draggingItem; // Для временного хранения перетаскиваемого предмета

    // Размеры сетки
    static const int GRID_WIDTH = 7;
    static const int GRID_HEIGHT = 6;
    static const int SLOT_SIZE = 32;
    static const int EQUIP_SLOT_SIZE = 64;
    sf::Vector2f INVENTORY_SLOTS_SIZE;

    // Позиция начала сетки инвентаря
    sf::Vector2f inventoryGridStartPos = { 64, 0 };

    // Добавьте этот метод для вычисления позиции слота
    sf::Vector2f getSlotPosition(int x, int y) const;

    // Добавьте для хранения информации о hover
    int hoveredGridX = -1;
    int hoveredGridY = -1;

    // Текстура для фона инвентаря
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    // Текстуры для слотов
    sf::Texture slotTexture;
    std::vector<sf::Sprite> slotSprites;
    sf::Texture equslotTexture;
    sf::Sprite equslotSprites;
    sf::Texture hoverTexture;
    sf::Sprite hoverSprite; // Sprite to show when hovering over a slot
    bool isHoveringSlot = false;
    sf::Vector2f hoverSlotPosition;
    bool wasHovering = false;

    sf::RectangleShape quickAccessSlots[9]; // 9 слотов быстрого доступа
    sf::Sprite quickAccessSlotSprites[9];
    std::unique_ptr<Item> quickAccessItems[9]; // Предметы в слотах быстрого доступа
    const float QUICK_ACCESS_SLOT_SIZE = 32.f;
    sf::Vector2f quickAccessStartPos = { 288, 0 }; // Начальная позиция слотов быстрого доступа
    sf::Texture quickSlotActiveTexture; // Текстура для активного слота
    sf::Sprite quickSlotActiveSprite; // Спрайт для подсветки активного слота

    void equipWeaponFromQuickSlot(int slotIndex);
    void unequipCurrentWeapon();
    int lastEquippedWeaponSlot = -1; // Для запоминания последнего экипированного слота

    bool isVisible_ = false;
    float pickupRadius_ = 32.0f; // радиус сбора предметов
    sf::Sprite outlineSprite; // Для outline эффекта

    void loadTextures(); // Загрузка текстур

    sf::Text pickupHintText;
    sf::Font font;

    void useQuickAccessItem(int index);
    void initSlots();
    void checkItemPickup();
    void checkItemDrop();
    void checkSlotHover();

    void updateTooltip(const sf::Vector2f& mousePos);
    void createTooltipContent(Item* item, const sf::Vector2f& mousePos);
    void createEmptySlotTooltip(const std::string& slotName, const sf::Vector2f& mousePos);
    void setupTooltipVisuals(const std::string& text, const sf::Vector2f& mousePos);
    sf::Text tooltipText;
    sf::RectangleShape tooltipBackground;
    bool showTooltip = false;
    sf::Vector2f tooltipPosition;
};