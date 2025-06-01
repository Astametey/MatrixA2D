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

    void scaleToSize(float width, float height) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        if (bounds.width > 0 && bounds.height > 0) {  // Защита от деления на 0
            sprite.setScale(width / bounds.width, height / bounds.height);
        }
    }
    virtual void equip(Player& player) = 0;
    virtual void unequip(Player& player) = 0;
};

class ConsumableItem : public Item {
public:
    float healthRestore; // Количество восстанавливаемого здоровья
    bool isPotion; // Флаг, является ли предмет зельем
    sf::Texture texture;

    ConsumableItem(const std::string& name, float restoreAmount, bool potion = false) {
        type = ItemType::CONSUMABLE;
        this->name = name;
        healthRestore = restoreAmount;
        isPotion = potion;
        isStackable = true;  // расходники стекируемыми
        maxStackSize = 99;   // Максимальный размер стека
    }

    void equip(Player& player) override {
        // Для расходуемых предметов equip означает использование
        use(player);
    }

    void unequip(Player& player) override {
        // Ничего не делаем, так как предмет уже использован
    }

    void use(Player& player) {
        // Здесь логика использования предмета
        // Например, восстановление здоровья игрока
        // player.heal(healthRestore);

        // Можно добавить эффекты или звуки
        std::cout << "Used " << name << ", restored " << healthRestore << " health" << std::endl;

        // После использования предмет удаляется из инвентаря
        // Это обрабатывается в Inventory::useQuickAccessItem
    }
};

class ArmorItem : public Item {
public:
    Armor armor;

    ArmorItem(const Armor& armor, ItemType item_tipe) : armor(armor) {
        type = item_tipe;
        name = armor.name;
        sprite = armor.sprite;
        getRect = armor.sprite.getTextureRect();
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
};

class WeaponItem : public Item {
public:
    Weapon weapon;

    WeaponItem(const Weapon& weapon, ItemType item_tipe) : weapon(weapon) {
        type = item_tipe;
        name = weapon.name;
        sprite = weapon.sprite;
        getRect = weapon.sprite.getTextureRect();
    }

    void equip(Player& player) override {
        std::cout << "Attempting to equip weapon: " << name << "\n";
        isEquipped = true;
        player.setWeapon(weapon);
        std::cout << "Weapon equipped: " << player.isMeleeEquipped() << "\n";
    }

    void unequip(Player& player) override {
        isEquipped = false;
        player.unqWeapon();
        std::cout << "unequip" << std::endl;
    }

};

class Inventory {
public:
    Inventory(sf::RenderWindow& window, Player& player);
    std::vector<std::unique_ptr<Item>> groundItems;
    bool addItem(std::unique_ptr<Item> newItem);
    bool isSlotOccupied(const sf::Vector2f& position) const;
    void addGroundItem(std::unique_ptr<Item> item, const sf::Vector2f& position);
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

    void GroundItemsrender();
    void toggleVisibility();
    void tryPickupItem(const sf::Vector2f& playerPos);
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
};