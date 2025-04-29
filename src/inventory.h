#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#include "player.h"
#include <vector>
#include <memory>
#include <utility>

// Base class for all inventory items
class InventoryItem {
public:
    enum class Type {
        ARMOR,
        WEAPON,
        CONSUMABLE,
        MISC
    };

    InventoryItem(Type type, const std::string& name)
        : itemType(type), itemName(name), stackSize(1), maxStackSize(1) {}
    virtual ~InventoryItem() = default;

    Type getType() const { return itemType; }
    std::string getName() const { return itemName; }
    int getStackSize() const { return stackSize; }
    int getMaxStackSize() const { return maxStackSize; }

    void setStackSize(int size) {
        stackSize = (size <= maxStackSize) ? size : maxStackSize;
    }

    virtual bool isEquippable() const { return false; }
    virtual bool isUsable() const { return false; }
    virtual void use(Player* player) {}

    // For rendering in inventory
    virtual const sf::Sprite& getSprite() const {
        static sf::Sprite defaultSprite;
        return defaultSprite;
    }

protected:
    Type itemType;
    std::string itemName;
    int stackSize;
    int maxStackSize;
};

// Armor item wrapper
class ArmorItem : public InventoryItem {
public:
    ArmorItem(std::shared_ptr<Armor> armor, const std::string& slotType)
        : InventoryItem(Type::ARMOR, armor->name), armorData(armor), slotType(slotType) {
        maxStackSize = 1;
    }

    bool isEquippable() const override { return true; }
    std::shared_ptr<Armor> getArmor() const { return armorData; }
    std::string getSlotType() const { return slotType; }

    const sf::Sprite& getSprite() const override {
        return armorData->sprite;  // ќбратите внимание на -> вместо .
    }

private:
    std::shared_ptr<Armor> armorData;
    std::string slotType;
};

// Weapon item wrapper
class WeaponItem : public InventoryItem {
public:
    WeaponItem(const Weapon& weapon)
        : InventoryItem(Type::WEAPON, weapon.name), weaponData(weapon) {
        maxStackSize = 1;
        // ”бедимс€, что спрайт использует текстуру
        weaponData.sprite.setTexture(weaponData.texture);
    }

    const sf::Sprite& getSprite() const override {
        return weaponData.sprite;
    }

    bool isEquippable() const override { return true; }
    const Weapon& getWeapon() const { return weaponData; }
    Weapon::Type getWeaponType() const { return weaponData.type; }
    

private:
    Weapon weaponData;
};

// Inventory slot
struct InventorySlot {
    std::shared_ptr<InventoryItem> item;
    int quantity;

    InventorySlot() : item(nullptr), quantity(0) {}
    explicit InventorySlot(std::shared_ptr<InventoryItem> item, int quantity = 1)
        : item(item), quantity(quantity) {}

    bool isEmpty() const { return item == nullptr; }
    void clear() { item = nullptr; quantity = 0; }
};

class Inventory {
public:
    static const int INVENTORY_WIDTH = 10;
    static const int INVENTORY_HEIGHT = 5;
    static const int EQUIPMENT_SLOTS = 6; // head, body, legs, feet, melee, ranged

    Inventory() {
        // Initialize inventory grid
        items.resize(INVENTORY_HEIGHT, std::vector<InventorySlot>(INVENTORY_WIDTH));

        // Initialize equipment slots
        equipment.resize(EQUIPMENT_SLOTS);
    }

    // Add item to inventory
    bool addItem(std::shared_ptr<InventoryItem> item, int quantity = 1) {
        if (!item) return false;

        // Try to stack with existing items first
        if (item->getMaxStackSize() > 1) {
            for (auto& row : items) {
                for (auto& slot : row) {
                    if (!slot.isEmpty() &&
                        slot.item->getName() == item->getName() &&
                        slot.quantity < slot.item->getMaxStackSize()) {

                        int canAdd = slot.item->getMaxStackSize() - slot.quantity;
                        int addAmount = std::min(quantity, canAdd);

                        slot.quantity += addAmount;
                        quantity -= addAmount;

                        if (quantity == 0) return true;
                    }
                }
            }
        }

        // Find empty slot for remaining items
        for (auto& row : items) {
            for (auto& slot : row) {
                if (slot.isEmpty()) {
                    slot.item = item;
                    slot.quantity = std::min(quantity, item->getMaxStackSize());
                    quantity -= slot.quantity;

                    if (quantity == 0) return true;
                }
            }
        }

        return quantity == 0; // Return false if couldn't add all items
    }

    // Equip item from inventory
    bool equipItem(int inventoryX, int inventoryY) {
        if (inventoryX < 0 || inventoryX >= INVENTORY_WIDTH ||
            inventoryY < 0 || inventoryY >= INVENTORY_HEIGHT) {
            return false;
        }

        auto& slot = items[inventoryY][inventoryX];
        if (slot.isEmpty() || !slot.item->isEquippable()) {
            return false;
        }

        // Handle armor equipment
        if (auto armorItem = std::dynamic_pointer_cast<ArmorItem>(slot.item)) {
            int equipSlot = -1;
            std::string slotType = armorItem->getSlotType();

            if (slotType == "head") equipSlot = 0;
            else if (slotType == "body") equipSlot = 1;
            else if (slotType == "legs") equipSlot = 2;
            else if (slotType == "feet") equipSlot = 3;

            if (equipSlot != -1) {
                // Swap with currently equipped item
                auto oldItem = equipment[equipSlot].item;
                equipment[equipSlot].item = slot.item;
                equipment[equipSlot].quantity = 1;

                slot.item = oldItem;
                slot.quantity = oldItem ? 1 : 0;

                return true;
            }
        }
        // Handle weapon equipment
        else if (auto weaponItem = std::dynamic_pointer_cast<WeaponItem>(slot.item)) {
            int equipSlot = (weaponItem->getWeaponType() == Weapon::Type::MELEE) ? 4 : 5;

            // Swap with currently equipped weapon
            auto oldItem = equipment[equipSlot].item;
            equipment[equipSlot].item = slot.item;
            equipment[equipSlot].quantity = 1;

            slot.item = oldItem;
            slot.quantity = oldItem ? 1 : 0;

            return true;
        }

        return false;
    }

    // Unequip item to inventory
    bool unequipItem(int equipmentSlot) {
        if (equipmentSlot < 0 || equipmentSlot >= EQUIPMENT_SLOTS ||
            equipment[equipmentSlot].isEmpty()) {
            return false;
        }

        // Try to add to inventory
        if (addItem(equipment[equipmentSlot].item, 1)) {
            equipment[equipmentSlot].clear();
            return true;
        }

        return false;
    }

    // Get equipped items
    const std::vector<InventorySlot>& getEquipment() const {
        return equipment;
    }
    //  неконстантный метод
    std::vector<InventorySlot>& getEquipment() {
        return equipment;
    }

    // Get inventory grid
    const std::vector<std::vector<InventorySlot>>& getItems() const {
        return items;
    }
    // неконстантный метод
    std::vector<std::vector<InventorySlot>>& getItems() {
        return items;
    }

    // Apply equipment stats to player
    void applyEquipment(Player& player) {
        // Head armor
        if (!equipment[0].isEmpty()) {
            if (auto armorItem = std::dynamic_pointer_cast<ArmorItem>(equipment[0].item)) {
                player.setArmorHead(*armorItem->getArmor()); // –азыменовываем shared_ptr
            }
        }

        // Body armor
        if (!equipment[1].isEmpty()) {
            if (auto armorItem = std::dynamic_pointer_cast<ArmorItem>(equipment[1].item)) {
                player.setArmorBody(*armorItem->getArmor());
            }
        }

        // Legs armor
        if (!equipment[1].isEmpty()) {
            if (auto armorItem = std::dynamic_pointer_cast<ArmorItem>(equipment[2].item)) {
                player.setArmorLegs(*armorItem->getArmor());
            }
        }

        // Shoes armor
        if (!equipment[1].isEmpty()) {
            if (auto armorItem = std::dynamic_pointer_cast<ArmorItem>(equipment[3].item)) {
                player.setArmorShoes(*armorItem->getArmor());
            }
        }

        // Melee weapon (слот 4)
        if (!equipment[4].isEmpty()) {
            if (auto weaponItem = std::dynamic_pointer_cast<WeaponItem>(equipment[4].item)) {
                player.setMeleeWeapon(weaponItem->getWeapon());
                std::cout << "Melee weapon equipped: " << weaponItem->getName() << std::endl;
            }
        }

        // Ranged weapon (слот 5)
        if (!equipment[5].isEmpty()) {
            if (auto weaponItem = std::dynamic_pointer_cast<WeaponItem>(equipment[5].item)) {
                player.setRangedWeapon(weaponItem->getWeapon());
            }
        }

        // ... аналогично дл€ остального снар€жени€
    }

    std::vector<InventorySlot>& getEquipmentNonConst() {
        return equipment;
    }

private:
    std::vector<std::vector<InventorySlot>> items; // Inventory grid
    std::vector<InventorySlot> equipment; // Equipped items
};



#endif // _INVENTORY_H_