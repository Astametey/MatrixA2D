//----------------��� ���������������� ��������
//����� ���:
//auto hair = std::make_shared<Armor>();
//hair->name = "hair";
//hair->defense = 2.0f;
//if (!hair->loadTexture("resources/Entities/Mikacharacter-Sheet.png")) {
//    std::cerr << "Failed to load helmet texture!" << std::endl;
//}
//hair->sprite.setTextureRect(sf::IntRect(0, 192, 16, 32));//�� ����������� � � ��� ������ ��� ������ � ���������
//
//// ������������� ������
//Weapon ironSword; 
//ironSword.name = "Iron Sword"; 
//ironSword.type = Weapon::Type::MELEE; 
//ironSword.damage = 2.0f; 
//ironSword.criticalChance = 0.3f; 
//if (!ironSword.loadTexture("resources/Entities/sword.png")) { 
//    std::cerr << "������: �� ������� ��������� �������� ����!" << std::endl; 
//} 
//else { 
//    std::cout << "�������� ���� ������� ���������. ������: " 
//        << ironSword.texture.getSize().x << "x" 
//        << ironSword.texture.getSize().y << std::endl; 
//} 
//ironSword.sprite.setTextureRect(sf::IntRect(0, 0, 16, 32)); 
// 
// // ��������� �������� � ���������
//playerInventory.addItem(std::make_shared<ArmorItem>(hair, "head"));
//playerInventory.addItem(std::make_shared<WeaponItem>(ironSword));
// 
// 
// 
// 
// 
//

#ifndef _RESOURCE_INIT_H_
#define _RESOURCE_INIT_H_

#include "view.h"
#include "level.h"
#include "enemies.h"
#include "inventory.h"

// ������� ��� "�����������" � ����������� �����
void respawnEnemy(Enemy& enemy, Level* level) {
    // ����� ��������� �����
    enemy.setHealth(enemy.maxHealth); // ��������������� ��������
    enemy.takeDamage(0);
    // ��������� ��������� ���������
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> x_dist(50.0f, (LEVEL_WIDTH * level->tileWidth)- 50.0f);
    std::uniform_real_distribution<> y_dist(50.0f, (LEVEL_HEIGHT * level->tileHeight) - 50.0f);

    sf::Vector2f dist;
    dist.x = x_dist(gen);
    dist.y = y_dist(gen);


    // ����������� �����
    enemy.setPosition(dist);

    //������� ������������ ����� ������� (��� ������ ���������� �����)

}


//________GLOBAL VALUE________

bool inventoryOpen = false;
// ���������� ���������� ��� ��������������
bool isDragging = false;
int dragFromX = -1, dragFromY = -1;
bool isFromEquipment = false;
int dragFromEquipmentSlot = -1;
std::shared_ptr<InventoryItem> draggedItem;
int draggedQuantity = 0;
sf::Vector2f dragOffset;

Level test_level;

//--Entity
std::vector<std::unique_ptr<Enemy>> enemies;

//--Player
Player player(32, 32,
    16, 16,  // collision width/height (������� ��������)
    16, 32,  // visual width/height (������ ��������)
    test_level);

Inventory playerInventory;

//----Timers
sf::Clock main_clock;
float deltaTime;

//----Fonts
sf::Font base_font;

void drawInventory(sf::RenderWindow* win, Inventory& inventory);


//������������� ��������
void resource_init(sf::RenderWindow* l_win)
{
    //fonts
    if (!base_font.loadFromFile("resources/Fonts/Times_New_Roman.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
    }

    test_level.LoadFromFile("resources//levels//test_map_MatrixA2D.tmx", "resources//levels//forest_tile_set.png", l_win);
    main_view.reset(sf::FloatRect(0, 0, 640, 360));
    //Player
    // Load armor textures

    // ������������� ���������
    // ������� ��������

    auto shirts = std::make_shared<Armor>();
    shirts->name = "shirts";
    shirts->defense = 1.0f;
    if (!shirts->loadTexture("resources/Entities/shirts.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    shirts->sprite.setTextureRect(sf::IntRect(0, 0, 16, 32));


    // ������������� ������
    Weapon ironSword;
    ironSword.name = "Iron Sword";
    ironSword.type = Weapon::Type::MELEE;
    ironSword.damage = 2.0f;
    ironSword.criticalChance = 0.3f;
    if (!ironSword.loadTexture("resources/Entities/sword.png")) {
        std::cerr << "������: �� ������� ��������� �������� ����!" << std::endl;
    }
    ironSword.sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
    //ironSword.sprite.setOrigin(9, 22);


    // ��������� �������� � ���������
    playerInventory.addItem(std::make_shared<ArmorItem>(shirts, "body"));
    //playerInventory.addItem(std::make_shared<ArmorItem>(shoes, "feet"));


    playerInventory.addItem(std::make_shared<WeaponItem>(ironSword));

    // ��������� �������� �� ���������
    playerInventory.equipItem(0, 2); // ��������� � ���� ������


    // ��������� ���������� � ������
    playerInventory.applyEquipment(player);


    // ������� ������, ���� �� ��� ��� ��������
    enemies.clear();

    // ��������� ������ � ������� ������������
    enemies.push_back(std::make_unique<Enemy>(100.0f, 400.0f, 32.0f, 32.0f, test_level));  // ���� 1
    enemies.push_back(std::make_unique<Enemy>(100.0f, 400.0f, 32.0f, 32.0f, test_level));  // ���� 2
    enemies.push_back(std::make_unique<Enemy>(100.0f, 400.0f, 32.0f, 32.0f, test_level));  // ���� 3
    
    // �������� ������� ������, ������� �����, � ������� ������������
    

    if (!enemies.empty()) {
        Enemy* enemy = enemies[0].get(); // �������� ����� ��������� �� ������ Enemy
        enemy->Set_type_enemy("orc"); 
        enemy = enemies[1].get();
        enemy->Set_type_enemy("orc");
        enemy = enemies[2].get();
        enemy->Set_type_enemy("orc");
    }

}

void startDragging(int x, int y, bool fromEquipment, int slotIndex,
    std::shared_ptr<InventoryItem> item, int quantity,
    sf::Vector2f mousePos, sf::Vector2f itemPos) {
    isDragging = true;
    isFromEquipment = fromEquipment;
    draggedItem = item;
    draggedQuantity = quantity;
    dragOffset = mousePos - itemPos;

    if (fromEquipment) {
        dragFromEquipmentSlot = slotIndex;
    }
    else {
        dragFromX = x;
        dragFromY = y;
    }
}

void resetDragState() {
    isDragging = false;
    isFromEquipment = false;
    draggedItem = nullptr;
    draggedQuantity = 0;
    dragFromX = -1;
    dragFromY = -1;
    dragFromEquipmentSlot = -1;
}

void stopDragging(Inventory& inventory) {
    if (!isDragging) return;

    // ���� ������� �� ��� ������� � ������ ����, ���������� ��� �������
    if (isFromEquipment) {
        inventory.getEquipmentNonConst()[dragFromEquipmentSlot].item = draggedItem;
        inventory.getEquipmentNonConst()[dragFromEquipmentSlot].quantity = draggedQuantity;
    } 
    else {
        inventory.getItems()[dragFromY][dragFromX].item = draggedItem;
        inventory.getItems()[dragFromY][dragFromX].quantity = draggedQuantity;
    }

    resetDragState();
}

void drawDraggedItem(sf::RenderWindow* win) {
    if (!isDragging || !draggedItem) return;

    sf::Sprite itemSprite = draggedItem->getSprite();
    sf::Vector2f mousePos = win->mapPixelToCoords(sf::Mouse::getPosition(*win));
    itemSprite.setPosition(mousePos - dragOffset);

    // ��������������� ��� � ���������
    float scaleX = 40.0f / itemSprite.getLocalBounds().width;
    float scaleY = 40.0f / itemSprite.getLocalBounds().height;
    itemSprite.setScale(scaleX, scaleY);

    // ���������������� ��� ��������������
    itemSprite.setColor(sf::Color(255, 255, 255, 180));

    win->draw(itemSprite);
}

bool canEquipToSlot(std::shared_ptr<InventoryItem> item, int slotIndex) {
    if (!item->isEquippable()) return false;

    if (auto armor = std::dynamic_pointer_cast<ArmorItem>(item)) {
        switch (slotIndex) {
        case 0: return armor->getSlotType() == "head";
        case 1: return armor->getSlotType() == "body";
        case 2: return armor->getSlotType() == "legs";
        case 3: return armor->getSlotType() == "feet";
        default: return false;
        }
    }
    else if (auto weapon = std::dynamic_pointer_cast<WeaponItem>(item)) {
        if (slotIndex == 4) return weapon->getWeaponType() == Weapon::Type::MELEE;
        if (slotIndex == 5) return weapon->getWeaponType() == Weapon::Type::RANGED;
    }

    return false;
}

void handleInventoryMouseEvents(sf::Event& event, sf::RenderWindow* win, Inventory& inventory) {
    if (!inventoryOpen) return;

    // ����������� ���������� ���� � ���������� ���� (����� ������/�������)
    sf::Vector2f mousePos = win->mapPixelToCoords(sf::Mouse::getPosition(*win), win->getView());

    // ��� �������: ������� ���������� ����
    //std::cout << "Mouse position: " << mousePos.x << ", " << mousePos.y << std::endl;

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        // ��������� ����� ���������� (������� ���)
        for (int i = 0; i < inventory.getEquipment().size(); ++i) {
            sf::FloatRect slotRect(110 + i * 55, 60, 50, 50);
            if (slotRect.contains(mousePos)) {
                std::cout << "Clicked equipment slot " << i << std::endl;
                auto& slot = inventory.getEquipmentNonConst()[i];
                if (!slot.isEmpty()) {
                    startDragging(0, 0, true, i, slot.item, slot.quantity, mousePos,
                        sf::Vector2f(115 + i * 55, 65));
                    slot.clear();
                }
                return;
            }
        }

        // ��������� ������� ����� ���������
        auto& items = inventory.getItems();
        for (int y = 0; y < items.size(); ++y) {
            for (int x = 0; x < items[y].size(); ++x) {
                sf::FloatRect slotRect(110 + x * 55, 130 + y * 55, 50, 50);
                if (slotRect.contains(mousePos)) {
                    std::cout << "Clicked inventory slot " << x << "," << y << std::endl;
                    if (!items[y][x].isEmpty()) {
                        startDragging(x, y, false, -1, items[y][x].item, items[y][x].quantity,
                            mousePos, sf::Vector2f(115 + x * 55, 135 + y * 55));
                        items[y][x].clear();
                    }
                    return;
                }
            }
        }
    }

    // ���������� ������ ���� - ������� ��������� �������
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && isDragging) {
        bool placed = false;

        // ������� ��������� � ���� ����������
        for (int i = 0; i < inventory.getEquipment().size(); ++i) {
            sf::FloatRect slotRect(110 + i * 55, 60, 50, 50);
            if (slotRect.contains(mousePos)) {
                if (canEquipToSlot(draggedItem, i)) {
                    auto& slot = inventory.getEquipmentNonConst()[i];
                    if (!slot.isEmpty()) {
                        // ���� � ����� ��� ���� �������, ������ �������
                        if (isFromEquipment) {
                            inventory.getEquipmentNonConst()[dragFromEquipmentSlot].item = slot.item;
                            inventory.getEquipmentNonConst()[dragFromEquipmentSlot].quantity = slot.quantity;
                        }
                        else {
                            inventory.getItems()[dragFromY][dragFromX].item = slot.item;
                            inventory.getItems()[dragFromY][dragFromX].quantity = slot.quantity;
                        }
                    }
                    slot.item = draggedItem;
                    slot.quantity = draggedQuantity;
                    placed = true;
                }
                break;
            }
        }

        // ������� ��������� � ������� ���� ���������
        if (!placed) {
            auto& items = inventory.getItems();
            for (int y = 0; y < items.size(); ++y) {
                for (int x = 0; x < items[y].size(); ++x) {
                    sf::FloatRect slotRect(110 + x * 55, 130 + y * 55, 50, 50);
                    if (slotRect.contains(mousePos)) {
                        if (items[y][x].isEmpty()) {
                            items[y][x].item = draggedItem;
                            items[y][x].quantity = draggedQuantity;
                        }
                        else if (items[y][x].item->getName() == draggedItem->getName() &&
                            items[y][x].quantity < items[y][x].item->getMaxStackSize()) {
                            // ������� ������� ����������� ��������
                            int canAdd = items[y][x].item->getMaxStackSize() - items[y][x].quantity;
                            int addAmount = std::min(draggedQuantity, canAdd);

                            items[y][x].quantity += addAmount;
                            draggedQuantity -= addAmount;

                            if (draggedQuantity > 0) {
                                // ���� �������� ��������, ��������� �� ����������������
                                draggedQuantity = addAmount;
                                placed = true;
                            }
                        }
                        else {
                            // ���� �������� ������, ������ �������
                            auto tempItem = items[y][x].item;
                            int tempQuantity = items[y][x].quantity;

                            items[y][x].item = draggedItem;
                            items[y][x].quantity = draggedQuantity;

                            draggedItem = tempItem;
                            draggedQuantity = tempQuantity;

                            if (isFromEquipment) {
                                startDragging(0, 0, true, dragFromEquipmentSlot,
                                    draggedItem, draggedQuantity, mousePos,
                                    sf::Vector2f(115 + x * 55, 135 + y * 55));
                                return;
                            }
                            else {
                                startDragging(x, y, false, -1, draggedItem, draggedQuantity,
                                    mousePos, sf::Vector2f(115 + x * 55, 135 + y * 55));
                                return;
                            }
                        }
                        placed = true;
                        break;
                    }
                }
                if (placed) break;
            }
        }

        if (!placed) {
            stopDragging(inventory);
        }
        else {
            resetDragState();
        }
    }
}


;
#endif //_RESOURCE_INIT_H_