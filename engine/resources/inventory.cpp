#include "Inventory.h"
#include <iostream>

Inventory::Inventory(sf::RenderWindow& window, Player& player)
    : window(window), player(player) {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");
    loadTextures();
    initSlots();

    if (!font.loadFromFile("resources/fonts/arial.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
    }

    pickupHintText.setFont(font);
    pickupHintText.setString("Press E to pickup");
    pickupHintText.setCharacterSize(16);
    pickupHintText.setFillColor(sf::Color::White);

    pickupHintText.setScale(0.5f, 0.5f); // Уменьшаем в 2 раза

    tooltipText.setFont(font);
    tooltipText.setCharacterSize(14);
    tooltipText.setFillColor(sf::Color::White);

    tooltipBackground.setFillColor(sf::Color(30, 30, 30, 240));
    tooltipBackground.setOutlineColor(sf::Color(100, 100, 100));
    tooltipBackground.setOutlineThickness(1.f);
    tooltipBackground.setSize(sf::Vector2f(150, 80));
}

void Inventory::initSlots() {
    // Initialize inventory slots
    INVENTORY_SLOTS_SIZE.x = GRID_WIDTH * SLOT_SIZE;
    INVENTORY_SLOTS_SIZE.y = GRID_HEIGHT * SLOT_SIZE;
    inventoryBackground.setSize(sf::Vector2f(INVENTORY_SLOTS_SIZE.x, INVENTORY_SLOTS_SIZE.y));
    inventoryBackground.setPosition(EQUIP_SLOT_SIZE, 0);
    inventoryBackground.setTexture(&backgroundTexture);

    helmetSlot.setSize(sf::Vector2f(64, 64));
    helmetSlot.setPosition(0, 0);
    helmetSlot.setTexture(&backgroundTexture);

    bodySlot.setSize(sf::Vector2f(64, 64));
    bodySlot.setPosition(0, 64);
    bodySlot.setTexture(&backgroundTexture);

    legsSlot.setSize(sf::Vector2f(64, 64));
    legsSlot.setPosition(0, 128);
    legsSlot.setTexture(&backgroundTexture);

    shoesSlot.setSize(sf::Vector2f(64, 64));
    shoesSlot.setPosition(0, 192);
    shoesSlot.setTexture(&backgroundTexture);

    // Создаем сетку 7x6 
    slotSprites.clear();
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            sf::Sprite slot;
            slot.setTexture(slotTexture);
            slot.setPosition(getSlotPosition(x, y));
            slotSprites.push_back(slot);
        }
    }

    for (int i = 0; i < 9; ++i) {
        quickAccessSlots[i].setSize(sf::Vector2f(QUICK_ACCESS_SLOT_SIZE, QUICK_ACCESS_SLOT_SIZE));
        quickAccessSlots[i].setPosition(quickAccessStartPos.x + i * QUICK_ACCESS_SLOT_SIZE,
            quickAccessStartPos.y);
        quickAccessSlots[i].setTexture(&slotTexture);

        quickAccessSlotSprites[i].setTexture(slotTexture);
        quickAccessSlotSprites[i].setPosition(quickAccessSlots[i].getPosition());
    }
    activeQuickSlot = 0;
    
}

bool Inventory::addItem(std::unique_ptr<Item> newItem) {
    if (!newItem) return false;

    // 1. Сначала проверяем слоты быстрого доступа
    if (newItem->isStackable) {
        // Проверяем заполненные слоты быстрого доступа для объединения
        for (int i = 0; i < 9; ++i) {
            if (quickAccessItems[i] && quickAccessItems[i]->canStackWith(*newItem)) {
                int canAdd = quickAccessItems[i]->maxStackSize - quickAccessItems[i]->stackSize;
                int willAdd = std::min(canAdd, newItem->stackSize);

                quickAccessItems[i]->stackSize += willAdd;
                newItem->stackSize -= willAdd;

                if (newItem->stackSize <= 0) {
                    return true; // Весь предмет добавлен в быстрый слот
                }
            }
        }
    }

    // 2. Проверяем основной инвентарь для объединения
    if (newItem->isStackable) {
        for (auto& existingItem : items) {
            if (existingItem && existingItem->canStackWith(*newItem)) {
                int canAdd = existingItem->maxStackSize - existingItem->stackSize;
                int willAdd = std::min(canAdd, newItem->stackSize);

                existingItem->stackSize += willAdd;
                newItem->stackSize -= willAdd;

                if (newItem->stackSize <= 0) {
                    return true; // Весь предмет добавлен в существующий стек
                }
            }
        }
    }

    // 3. Если предмет не стекнулся никуда, пробуем добавить в свободный быстрый слот
    if (canPlaceInQuickSlot(newItem->type)) {
        for (int i = 0; i < 9; ++i) {
            if (!quickAccessItems[i]) {
                quickAccessItems[i] = std::move(newItem);
                quickAccessItems[i]->scaleToSize(QUICK_ACCESS_SLOT_SIZE, QUICK_ACCESS_SLOT_SIZE);
                quickAccessItems[i]->setPosition(quickAccessSlots[i].getPosition());
                return true;
            }
        }
    }

    // 4. Добавление в основной инвентарь с новой проверкой
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            sf::Vector2f slotPos = getSlotPosition(x, y);

            // Новая проверка - ищем предметы с такой же позицией
            bool slotOccupied = false;
            for (const auto& item : items) {
                if (item && item->sprite.getPosition() == slotPos) {
                    slotOccupied = true;
                    break;
                }
            }

            if (!slotOccupied) {
                newItem->setPosition(slotPos);
                newItem->scaleToSize(SLOT_SIZE, SLOT_SIZE);

                // Проверяем дубликаты перед добавлением
                bool duplicateFound = false;
                for (const auto& item : items) {
                    if (item && item->sprite.getGlobalBounds().contains(slotPos)) {
                        duplicateFound = true;
                        break;
                    }
                }

                if (!duplicateFound) {
                    items.push_back(std::move(newItem));
                    return true;
                }
            }
        }
    }

    return false; // Инвентарь полон
}
bool Inventory::isSlotOccupied(const sf::Vector2f& position) const {
    // Проверка быстрых слотов
    for (int i = 0; i < 9; ++i) {
        if (quickAccessItems[i] &&
            quickAccessItems[i]->sprite.getPosition() == position) {
            return true;
        }
    }

    // Проверка основного инвентаря
    for (const auto& item : items) {
        if (item && item->sprite.getPosition() == position) {
            return true;
        }
    }

    return false;
}

sf::Vector2f Inventory::getSlotPosition(int x, int y) const {
    return sf::Vector2f(
        inventoryGridStartPos.x + x * SLOT_SIZE,
        inventoryGridStartPos.y + y * SLOT_SIZE
    );
}


void Inventory::toggleVisibility() {
    isVisible_ = !isVisible_;

    // Можно добавить звук открытия/закрытия
    // soundManager.playSound("inventory_open.wav");
}
bool Inventory::isVisible() const {
    return isVisible_;
}
void Inventory::tryPickupItem(const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Item>>& groundItems) {
    for (auto it = groundItems.begin(); it != groundItems.end(); ) {
        float distance = std::sqrt(
            std::pow(playerPos.x - (*it)->sprite.getPosition().x, 2) +
            std::pow(playerPos.y - (*it)->sprite.getPosition().y, 2));

        if (distance <= pickupRadius_) {
            if (addItem(std::move(*it))) {
                it = groundItems.erase(it);
            }
            else {
                ++it;
            }
        }
        else {
            ++it;
        }
    }
}
bool Inventory::tryMergeStacks(Item& source, Item& target) {
    if (!source.isStackable || !target.isStackable) return false;
    if (!source.canStackWith(target)) return false;
    if (target.stackSize >= target.maxStackSize) return false;

    int canAdd = target.maxStackSize - target.stackSize;
    int willAdd = std::min(canAdd, source.stackSize);

    target.stackSize += willAdd;
    source.stackSize -= willAdd;

    // Возвращаем true если source полностью объединен
    return (source.stackSize <= 0);
}
void Inventory::loadTextures() {
    // Загрузка текстур (добавьте свои пути)
    if (!backgroundTexture.loadFromFile("resources/textures/UI/RectangleBox_96x96.png")) {
        // Обработка ошибки
    }
    backgroundSprite.setTexture(backgroundTexture);

    if (!slotTexture.loadFromFile("resources/textures/UI/ItemBox_24x24.png")) {
        // Обработка ошибки
    }

    if (!hoverTexture.loadFromFile("resources/textures/UI/HighlightSlot_26x26.png")) {
        // Обработка ошибки
    }
    hoverSprite.setTexture(hoverTexture);
    quickSlotActiveSprite.setTexture(hoverTexture);
    quickSlotActiveSprite.setScale(
        QUICK_ACCESS_SLOT_SIZE / hoverTexture.getSize().x,
        QUICK_ACCESS_SLOT_SIZE / hoverTexture.getSize().y
    );

    if (!equslotTexture.loadFromFile("resources/textures/UI/BlankEquipmentIcons_20x17.png")) {
        // Обработка ошибки
    }
    equslotSprites.setTexture(equslotTexture);
    equslotSprites.setScale(2, 2);
    

    // Создание слотов (пример для 20 слотов)
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 6; ++j)
        {
            sf::Sprite slot;
            slot.setTexture(slotTexture);
            slot.setPosition(74 + (i * 32), (j * 32));
            slotSprites.push_back(slot);
        };
    }

    
}

void Inventory::render() {
    // Draw inventory background and slots first
    window.draw(inventoryBackground);
    equslotSprites.setTextureRect(sf::IntRect(43, 16, 14, 18));
    equslotSprites.setPosition(helmetSlot.getPosition().x + 18, helmetSlot.getPosition().y + 14);
    window.draw(helmetSlot);
    window.draw(equslotSprites);

    equslotSprites.setTextureRect(sf::IntRect(1, 0, 18, 16));
    equslotSprites.setPosition(bodySlot.getPosition().x + 14, bodySlot.getPosition().y + 15);
    window.draw(bodySlot);
    window.draw(equslotSprites);

    window.draw(legsSlot);

    equslotSprites.setTextureRect(sf::IntRect(3, 16, 13, 18));
    equslotSprites.setPosition(shoesSlot.getPosition().x + 18, shoesSlot.getPosition().y + 14);
    window.draw(shoesSlot);
    window.draw(equslotSprites);

    


    // Draw slot sprites
    for (const auto& slot : slotSprites) {
        window.draw(slot);
    }

    // Draw inventory items (not being dragged)
    for (auto& item : items) {
        if (!item->isDragging) {
            window.draw(item->sprite);
        }
    }

    // Draw equipped items
    if (equippedHelmet) window.draw(equippedHelmet->sprite);
    if (equippedBody) window.draw(equippedBody->sprite);
    if (equippedLegs) window.draw(equippedLegs->sprite);
    if (equippedShoes) window.draw(equippedShoes->sprite);

    // Отрисовка предметов в инвентаре с количеством
    for (auto& item : items) {
        window.draw(item->sprite);

        if (item->isStackable && item->stackSize > 1) {
            sf::Text countText;
            countText.setFont(font);
            countText.setString(std::to_string(item->stackSize));
            countText.setCharacterSize(12);
            countText.setFillColor(sf::Color::White);
            countText.setPosition(
                item->sprite.getPosition().x + SLOT_SIZE - 15,
                item->sprite.getPosition().y + SLOT_SIZE - 15
            );
            window.draw(countText);
        }
    }

    // Draw highlight if hovering
    if (isHoveringSlot) {
        bool isEquipmentSlot = (hoverSlotPosition == helmetSlot.getPosition() ||
            hoverSlotPosition == bodySlot.getPosition() ||
            hoverSlotPosition == legsSlot.getPosition() ||
            hoverSlotPosition == shoesSlot.getPosition());

        if (isEquipmentSlot) {
            // Центрируем выделение для экипировки
            float size = 64.0f;
            hoverSprite.setPosition(
                hoverSlotPosition.x + (size - hoverSprite.getGlobalBounds().width) / 2,
                hoverSlotPosition.y + (size - hoverSprite.getGlobalBounds().height) / 2
            );
        }
        else {
            // Обычные слоты - без смещения
            hoverSprite.setPosition(hoverSlotPosition);
        }

        window.draw(hoverSprite);
    }

    // Draw dragged item on top of everything
    for (auto& item : items) {
        if (item->isDragging) {
            window.draw(item->sprite);
        }
    }

    // В самом конце - перетаскиваемый предмет
    if (draggingItem) {
        window.draw(draggingItem->sprite);
    }

    if (showTooltip) {
        window.draw(tooltipBackground);
        window.draw(tooltipText);
    }
}
void Inventory::renderUI() {
    // Рендерим слоты быстрого доступа
    for (int i = 0; i < 9; ++i) {
        // Фон слота
        window.draw(quickAccessSlotSprites[i]);

        // Номер слота (1-9)
        sf::Text slotNumber;
        slotNumber.setFont(font);
        slotNumber.setString(std::to_string(i + 1));
        slotNumber.setCharacterSize(12);
        slotNumber.setFillColor(sf::Color::White);
        slotNumber.setPosition(
            quickAccessSlots[i].getPosition().x + 2,
            quickAccessSlots[i].getPosition().y + 2
        );
        window.draw(slotNumber);

        // Подсветка активного слота
        if (i == activeQuickSlot) {
            quickSlotActiveSprite.setPosition(quickAccessSlots[i].getPosition());
            window.draw(quickSlotActiveSprite);

            // Дополнительная подсветка
            sf::RectangleShape highlight(quickAccessSlots[i].getSize());
            highlight.setPosition(quickAccessSlots[i].getPosition());
            highlight.setFillColor(sf::Color(255, 255, 255, 30));
            window.draw(highlight);
        }

        // Рендерим предмет в слоте (если есть)
        if (quickAccessItems[i]) {
            window.draw(quickAccessItems[i]->sprite);

            // Отображаем количество для стекируемых предметов
            if (quickAccessItems[i]->isStackable && quickAccessItems[i]->stackSize > 1) {
                sf::Text countText;
                countText.setFont(font);
                countText.setString(std::to_string(quickAccessItems[i]->stackSize));
                countText.setCharacterSize(12);
                countText.setFillColor(sf::Color::White);
                countText.setPosition(
                    quickAccessSlots[i].getPosition().x + QUICK_ACCESS_SLOT_SIZE - 15,
                    quickAccessSlots[i].getPosition().y + QUICK_ACCESS_SLOT_SIZE - 15
                );
                window.draw(countText);
            }
        }
        
    }

    if (draggingItem) {
        window.draw(draggingItem->sprite);
    }

}

void Inventory::update(float dt, sf::View& gui_view) {
    hoveredGridX = -1;
    hoveredGridY = -1;
    isHoveringSlot = false;

    if (isVisible_) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), gui_view);

        // Проверяем экипировочные слоты
        auto checkSlot = [&](const sf::RectangleShape& slot) {
            if (slot.getGlobalBounds().contains(mousePos)) {
                isHoveringSlot = true;
                hoverSlotPosition = slot.getPosition();
                return true;
            }
            return false;
        };

        checkSlot(helmetSlot) || checkSlot(bodySlot) || checkSlot(legsSlot) ||
            checkSlot(shoesSlot);

        for (int i = 0; i < 9 && !isHoveringSlot; ++i) {
            if (quickAccessSlots[i].getGlobalBounds().contains(mousePos)) {
                isHoveringSlot = true;
                hoverSlotPosition = quickAccessSlots[i].getPosition();
            }
        }

        // Проверяем сетку инвентаря
        for (int y = 0; y < GRID_HEIGHT && hoveredGridX == -1; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                sf::Vector2f slotPos = getSlotPosition(x, y);
                sf::FloatRect slotBounds(slotPos.x, slotPos.y, SLOT_SIZE, SLOT_SIZE);

                if (slotBounds.contains(mousePos)) {
                    hoveredGridX = x;
                    hoveredGridY = y;
                    hoverSlotPosition = slotPos;
                    isHoveringSlot = true;
                    break;
                }
            }
        }
    }
    if (!isVisible_) {
        showTooltip = false;
        return;
    }

    // Обновляем спрайт выделения
    if (isHoveringSlot) {
        bool isEquipment = (hoverSlotPosition == helmetSlot.getPosition() ||
            hoverSlotPosition == bodySlot.getPosition() ||
            hoverSlotPosition == legsSlot.getPosition() ||
            hoverSlotPosition == shoesSlot.getPosition());

        float scale = isEquipment ? 64.0f / hoverTexture.getSize().x :
            SLOT_SIZE / hoverTexture.getSize().x;
        hoverSprite.setScale(scale, scale);

        if (isEquipment) {
            hoverSprite.setPosition(hoverSlotPosition.x + (64 - hoverSprite.getGlobalBounds().width) / 2,
                hoverSlotPosition.y + (64 - hoverSprite.getGlobalBounds().height) / 2);
        }
        else {
            hoverSprite.setPosition(hoverSlotPosition);
        }
    }
}

void Inventory::handleMouseWheelScroll(const sf::Event& event) {
    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
        // Определяем направление прокрутки
        int direction = (event.mouseWheelScroll.delta > 0) ? -1 : 1;

        // Изменяем активный слот с учетом направления
        activeQuickSlot += direction;

        // Обеспечиваем циклический перебор слотов (0-8)
        if (activeQuickSlot < 0) {
            activeQuickSlot = 8;
        }
        else if (activeQuickSlot > 8) {
            activeQuickSlot = 0;
        }

        // Обновляем экипировку (если в слоте оружие)
        updateEquipmentAfterQuickSlotChange(activeQuickSlot);
    }
}

void Inventory::handleEvent(const sf::Event& event, sf::View& gui_view) {

    sf::View oldView = window.getView();
    window.setView(gui_view);

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            // Проверяем, не пытаемся ли мы взять предмет из инвентаря
            if (!handleItemPickup(event)) {
                // Если не взяли предмет, пробуем использовать активный слот
                //useActiveQuickSlotItem();
            }
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            useActiveQuickSlotItem();
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        handleItemDrop(event);
        if (!player.ifEqupWeapon)
        {
            updateEquipmentAfterQuickSlotChange(activeQuickSlot);
        }
        
    }
    else if (event.type == sf::Event::MouseMoved && draggingItem) {
        handleItemDrag(event);
    }
    else if (event.type == sf::Event::MouseWheelScrolled) {
        handleMouseWheelScroll(event);
    }
    else if (event.type == sf::Event::KeyPressed) {
        handleKeyPress(event);
    }

    if (!isVisible_) return;

    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y), gui_view);

        // Проверяем, наведена ли мышь на предмет
        Item* hoveredItem = nullptr;

        // Проверяем обычные слоты инвентаря
        for (auto& item : items) {
            if (item->sprite.getGlobalBounds().contains(mousePos)) {
                hoveredItem = item.get();
                break;
            }
        }

        // Проверяем слоты быстрого доступа
        if (!hoveredItem) {
            for (int i = 0; i < 9; ++i) {
                if (quickAccessItems[i] && quickAccessSlots[i].getGlobalBounds().contains(mousePos)) {
                    hoveredItem = quickAccessItems[i].get();
                    break;
                }
            }
        }

        // Проверяем экипированные предметы
        if (!hoveredItem) {
            if (equippedHelmet && helmetSlot.getGlobalBounds().contains(mousePos)) hoveredItem = equippedHelmet.get();
            else if (equippedBody && bodySlot.getGlobalBounds().contains(mousePos)) hoveredItem = equippedBody.get();
            else if (equippedLegs && legsSlot.getGlobalBounds().contains(mousePos)) hoveredItem = equippedLegs.get();
            else if (equippedShoes && shoesSlot.getGlobalBounds().contains(mousePos)) hoveredItem = equippedShoes.get();
        }

        updateTooltip(mousePos);
    }

    window.setView(oldView);
}

void Inventory::handleItemDrag(const sf::Event& event) {
    if (!draggingItem) return;

    sf::Vector2f mousePos = window.mapPixelToCoords(
        sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
    draggingItem->updateDragging(mousePos);

    // Для стекируемых предметов можно добавить отображение количества при перетаскивании
    if (draggingItem->isStackable && draggingItem->stackSize > 1) {
        sf::Text countText;
        countText.setFont(font);
        countText.setString(std::to_string(draggingItem->stackSize));
        countText.setCharacterSize(12);
        countText.setFillColor(sf::Color::White);
        countText.setPosition(
            mousePos.x + 10,
            mousePos.y + 10
        );
        window.draw(countText);
    }
}
bool Inventory::handleItemPickup(const sf::Event& event) {
    sf::Vector2f mousePos = window.mapPixelToCoords(
        sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    bool isShiftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);



    // Если Shift зажат - обрабатываем автоматическое перемещение
    if (isShiftPressed && event.mouseButton.button == sf::Mouse::Left) {
        bool handled = handleShiftClick(mousePos);
        if (handled) return true; // Если Shift+Клик обработан, выходим
        // Иначе продолжаем как обычный клик
    }

    bool itemPicked = false;

    // 1. Проверяем быстрые слоты
    for (int i = 0; i < 9; ++i) {
        if (quickAccessSlots[i].getGlobalBounds().contains(mousePos)) {
            if (quickAccessItems[i]) {
                // Если это оружие и оно экипировано - снимаем
                if (i == lastEquippedWeaponSlot &&
                    quickAccessItems[i]->type == ItemType::WEAPON) {
                    unequipCurrentWeapon();
                }

                draggingItem = std::move(quickAccessItems[i]);
                draggingItem->originalPosition = quickAccessSlots[i].getPosition();
                draggingItem->startDragging(mousePos);
                itemPicked = true;
                break;
            }
        }
    }
    if (itemPicked) return true;

    // 2. Проверяем экипировочные слоты
    auto checkEquipmentSlot = [&](std::unique_ptr<Item>& item, const sf::RectangleShape& slot) -> bool {
        if (slot.getGlobalBounds().contains(mousePos)) {
            if (item) {
                draggingItem = std::move(item);
                draggingItem->originalPosition = slot.getPosition();
                draggingItem->startDragging(mousePos);
                draggingItem->unequip(player);
                draggingItem->scaleToSize(SLOT_SIZE, SLOT_SIZE); // Уменьшаем размер при снятии
                return true;
            }
        }
        return false;
    };

    if (checkEquipmentSlot(equippedHelmet, helmetSlot) ||
        checkEquipmentSlot(equippedBody, bodySlot) ||
        checkEquipmentSlot(equippedLegs, legsSlot) ||
        checkEquipmentSlot(equippedShoes, shoesSlot)) {
        return true;
    }

    // 3. Проверяем обычные слоты
    for (auto it = items.begin(); it != items.end(); ++it) {
        if ((*it)->sprite.getGlobalBounds().contains(mousePos)) {
            draggingItem = std::move(*it);
            draggingItem->originalPosition = draggingItem->sprite.getPosition();
            draggingItem->startDragging(mousePos);
            items.erase(it);
            return true;
        }
    }
    return false;
}
bool Inventory::handleShiftClick(const sf::Vector2f& mousePos) {
    // Проверяем, кликнули ли по предмету в инвентаре
    for (auto it = items.begin(); it != items.end(); ++it) {
        if ((*it)->sprite.getGlobalBounds().contains(mousePos)) {
            ItemType itemType = (*it)->type;

            // Проверяем, является ли предмет броней
            if (isArmorType(itemType)) {
                std::unique_ptr<Item> item = std::move(*it);
                items.erase(it); // Сначала удаляем из контейнера
                return moveArmorToSlot(std::move(item), itemType);
            }
            return false; // Не броня - не обрабатываем
        }
    }

    // Проверяем быстрые слоты
    for (int i = 0; i < 9; ++i) {
        if (quickAccessItems[i] &&
            quickAccessSlots[i].getGlobalBounds().contains(mousePos)) {
            ItemType itemType = quickAccessItems[i]->type;

            if (isArmorType(itemType)) {
                std::unique_ptr<Item> item = std::move(quickAccessItems[i]);
                return moveArmorToSlot(std::move(item), itemType);
            }
            return false; // Не броня - не обрабатываем
        }
    }

    return false;
}
bool Inventory::moveArmorToSlot(std::unique_ptr<Item> item, ItemType armorType) {
    if (!item) return false;

    // Определяем целевой слот для этого типа брони
    sf::RectangleShape* targetSlot = nullptr;
    std::unique_ptr<Item>* equippedItem = nullptr;

    switch (armorType) {
    case ItemType::HELMET:
        targetSlot = &helmetSlot;
        equippedItem = &equippedHelmet;
        break;
    case ItemType::BODY:
        targetSlot = &bodySlot;
        equippedItem = &equippedBody;
        break;
    case ItemType::LEGS:
        targetSlot = &legsSlot;
        equippedItem = &equippedLegs;
        break;
    case ItemType::SHOES:
        targetSlot = &shoesSlot;
        equippedItem = &equippedShoes;
        break;
    default:
        return false;
    }

    // Если слот занят - делаем обмен
    if (*equippedItem) {
        // Снимаем текущую экипировку
        (*equippedItem)->unequip(player);

        // Перемещаем текущую экипировку в инвентарь
        std::unique_ptr<Item> oldItem = std::move(*equippedItem);
        oldItem->scaleToSize(SLOT_SIZE, SLOT_SIZE); // Уменьшаем размер

        // Экипируем новый предмет
        item->equip(player);
        item->scaleToSize(EQUIP_SLOT_SIZE, EQUIP_SLOT_SIZE); // Увеличиваем размер
        item->setPosition(targetSlot->getPosition());
        *equippedItem = std::move(item);

        // Старый предмет помещаем в инвентарь
        return addItemToFirstFreeSlot(std::move(oldItem));
    }
    else {
        // Слот свободен - просто экипируем
        item->equip(player);
        item->scaleToSize(EQUIP_SLOT_SIZE, EQUIP_SLOT_SIZE); // Увеличиваем размер
        item->setPosition(targetSlot->getPosition());
        *equippedItem = std::move(item);
        return true;
    }
}
bool Inventory::isArmorType(ItemType type) const {
    return type == ItemType::HELMET || type == ItemType::BODY ||
        type == ItemType::LEGS || type == ItemType::SHOES;
}
void Inventory::handleItemDrop(const sf::Event& event) {
    if (!draggingItem) return;
    // Проверяем не опустел ли предмет
    if (draggingItem->stackSize <= 0) {
        draggingItem.reset();
        return;
    }
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
    draggingItem->isDragging = false;

    // 1. Попытка поместить в быстрый слот
    if (tryPlaceInQuickSlot(mousePos)) return;

    // 2. Попытка экипировать
    if (tryEquipItem(mousePos)) return;

    // 3. Попытка поместить в обычный слот
    if (tryPlaceInRegularSlot(mousePos)) return;

    // 4. Если все попытки неудачны, возвращаем предмет в исходный слот экипировки
    if (isEquipmentSlotPosition(draggingItem->originalPosition)) {
        auto returnToSlot = [&](std::unique_ptr<Item>& slot, const sf::RectangleShape& slotShape) {
            if (slotShape.getPosition() == draggingItem->originalPosition) {
                slot = std::move(draggingItem);
                slot->equip(player);
                slot->scaleToSize(EQUIP_SLOT_SIZE, EQUIP_SLOT_SIZE);
                return true;
            }
            return false;
        };

        if (returnToSlot(equippedHelmet, helmetSlot) ||
            returnToSlot(equippedBody, bodySlot) ||
            returnToSlot(equippedLegs, legsSlot) ||
            returnToSlot(equippedShoes, shoesSlot)) {
            return;
        }
    }

    // 5. Если предмет не из слота экипировки, пытаемся найти любой свободный слот
    if (!addItemToFirstFreeSlot(std::move(draggingItem))) {
        // Если даже это не удалось, просто удаляем предмет (крайний случай)
        draggingItem.reset();
    }
}
void Inventory::handleKeyPress(const sf::Event& event) {
    // Обработка цифр 1-9 для выбора слотов
    if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9) {
        int slotIndex = event.key.code - sf::Keyboard::Num1; // Преобразуем в 0-8
        selectQuickSlot(slotIndex);
    }
    // Обработка цифр на NumPad (опционально)
    else if (event.key.code >= sf::Keyboard::Numpad1 && event.key.code <= sf::Keyboard::Numpad9) {
        int slotIndex = event.key.code - sf::Keyboard::Numpad1; // Преобразуем в 0-8
        selectQuickSlot(slotIndex);
    }
}
void Inventory::useActiveQuickSlotItem() {
    // Проверяем, есть ли предмет в активном слоте
    if (!quickAccessItems[activeQuickSlot]) return;

    Item* item = quickAccessItems[activeQuickSlot].get();

    // Используем только расходуемые предметы по клику
    if (item->type == ItemType::CONSUMABLE) {
        if (ConsumableItem* consumable = dynamic_cast<ConsumableItem*>(item)) {
            consumable->use(player);
            if (consumable->stackSize > 1)
            {
             consumable->stackSize -= 1;
            }
            else
            {
             quickAccessItems[activeQuickSlot].reset();
            }
           
            
            

            // Звук использования предмета
            // soundManager.playSound("item_use.wav");
        }
    }
    // Оружие автоматически экипируется при выборе слота (уже реализовано в updateEquipmentAfterQuickSlotChange)
}
void Inventory::selectQuickSlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex > 8) return;

    // Если выбираем текущий активный слот - ничего не делаем
    if (slotIndex == activeQuickSlot) return;

    // Снимаем текущее оружие, если выбираем пустой слот или другой тип предмета
    if (activeQuickSlot != -1 &&
        (!quickAccessItems[slotIndex] ||
            quickAccessItems[slotIndex]->type != ItemType::WEAPON)) {
        unequipCurrentWeapon();
    }

    // Устанавливаем новый активный слот
    activeQuickSlot = slotIndex;

    // Если в новом слоте оружие - экипируем его
    if (quickAccessItems[activeQuickSlot] &&
        quickAccessItems[activeQuickSlot]->type == ItemType::WEAPON) {
        equipWeaponFromQuickSlot(activeQuickSlot);
    }
}

bool Inventory::tryPlaceInQuickSlot(const sf::Vector2f& mousePos) {
    if (!draggingItem) return false;

    for (int i = 0; i < 9; ++i) {
        if (quickAccessSlots[i].getGlobalBounds().contains(mousePos)) {
            if (!canPlaceInQuickSlot(draggingItem->type)) {
                return false;
            }

            // Если слот содержит предмет, пытаемся объединить стеки
            if (quickAccessItems[i]) {
                if (tryMergeStacks(*draggingItem, *quickAccessItems[i])) {
                    // Если стек полностью объединен, очищаем draggingItem
                    if (draggingItem->stackSize <= 0) {
                        draggingItem.reset();
                    }
                    // Если слот опустел после объединения (маловероятно, но возможно)
                    if (quickAccessItems[i]->stackSize <= 0) {
                        quickAccessItems[i].reset();
                    }
                    return true;
                }
            }

            // Если слот пуст или объединение невозможно
            if (!quickAccessItems[i]) {
                quickAccessItems[i] = std::move(draggingItem);
                quickAccessItems[i]->scaleToSize(QUICK_ACCESS_SLOT_SIZE, QUICK_ACCESS_SLOT_SIZE);
                quickAccessItems[i]->setPosition(quickAccessSlots[i].getPosition());
                return true;
            }
            else {
                // Обмен предметами
                std::swap(quickAccessItems[i], draggingItem);
                quickAccessItems[i]->scaleToSize(QUICK_ACCESS_SLOT_SIZE, QUICK_ACCESS_SLOT_SIZE);
                quickAccessItems[i]->setPosition(quickAccessSlots[i].getPosition());
                draggingItem->scaleToSize(SLOT_SIZE, SLOT_SIZE);
                return false; // Продолжаем перетаскивание
            }
        }
    }
    return false;
}

bool Inventory::tryEquipItem(const sf::Vector2f& mousePos) {
    auto trySlot = [&](std::unique_ptr<Item>& slot, const sf::RectangleShape& slotShape, ItemType requiredType) {
        if (draggingItem->type == requiredType && slotShape.getGlobalBounds().contains(mousePos)) {
            std::unique_ptr<Item> oldItem;
            if (slot) {
                oldItem = std::move(slot);
                oldItem->unequip(player);
                oldItem->scaleToSize(SLOT_SIZE, SLOT_SIZE);
            }

            slot = std::move(draggingItem);
            slot->equip(player);
            slot->scaleToSize(EQUIP_SLOT_SIZE, EQUIP_SLOT_SIZE);
            slot->setPosition(slotShape.getPosition());

            if (oldItem && !addItemToFirstFreeSlot(std::move(oldItem))) {
                // Откат если не удалось вернуть старый предмет
                slot->unequip(player);
                draggingItem = std::move(slot);
                slot = std::move(oldItem);
                slot->equip(player);
                return false;
            }
            return true;
        }
        return false;
    };

    return trySlot(equippedHelmet, helmetSlot, ItemType::HELMET) ||
        trySlot(equippedBody, bodySlot, ItemType::BODY) ||
        trySlot(equippedLegs, legsSlot, ItemType::LEGS) ||
        trySlot(equippedShoes, shoesSlot, ItemType::SHOES);
}

bool Inventory::tryPlaceInRegularSlot(const sf::Vector2f& mousePos) {
    if (hoveredGridX == -1 || hoveredGridY == -1 || !draggingItem) return false;

    sf::Vector2f targetPos = getSlotPosition(hoveredGridX, hoveredGridY);

    // Ищем предмет в целевом слоте
    auto targetIt = std::find_if(items.begin(), items.end(),
        [&targetPos](const auto& item) {
            return item->sprite.getPosition() == targetPos;
        });

    if (targetIt != items.end()) {
        // Слот занят - пытаемся объединить стеки
        if (tryMergeStacks(*draggingItem, **targetIt)) {
            if (draggingItem->stackSize <= 0) {
                draggingItem.reset();
            }
            if ((*targetIt)->stackSize <= 0) {
                items.erase(targetIt);
            }
            return true;
        }

        // Если объединение невозможно - меняем местами
        std::unique_ptr<Item> temp = std::move(*targetIt);
        *targetIt = std::move(draggingItem);
        draggingItem = std::move(temp);

        (*targetIt)->setPosition(targetPos);
        if (draggingItem) {
            draggingItem->setPosition(draggingItem->originalPosition);
        }
        return false; // Продолжаем перетаскивание swapped item
    }
    else {
        // Слот свободен - помещаем предмет
        draggingItem->setPosition(targetPos);
        draggingItem->scaleToSize(SLOT_SIZE, SLOT_SIZE);
        items.push_back(std::move(draggingItem));
        return true;
    }
}

void Inventory::returnItemToOriginalPosition() {
    if (!draggingItem) return;
    // Проверяем не опустел ли предмет
    if (draggingItem->stackSize <= 0) {
        draggingItem.reset();
        return;
    }
    draggingItem->setPosition(draggingItem->originalPosition);

    // Определяем тип слота по оригинальной позиции
    if (isEquipmentSlotPosition(draggingItem->originalPosition)) {
        // Возвращаем в экипировочный слот
        auto returnToSlot = [&](std::unique_ptr<Item>& slot, const sf::RectangleShape& slotShape) {
            if (slotShape.getPosition() == draggingItem->originalPosition) {
                slot = std::move(draggingItem);
                slot->equip(player);
                slot->scaleToSize(EQUIP_SLOT_SIZE, EQUIP_SLOT_SIZE);
                return true;
            }
            return false;
        };

        returnToSlot(equippedHelmet, helmetSlot) ||
            returnToSlot(equippedBody, bodySlot) ||
            returnToSlot(equippedLegs, legsSlot) ||
            returnToSlot(equippedShoes, shoesSlot);
    }
    else if (isQuickSlotPosition(draggingItem->originalPosition)) {
        // Возвращаем в быстрый слот
        for (int i = 0; i < 9; ++i) {
            if (quickAccessSlots[i].getPosition() == draggingItem->originalPosition) {
                quickAccessItems[i] = std::move(draggingItem);
                quickAccessItems[i]->scaleToSize(QUICK_ACCESS_SLOT_SIZE, QUICK_ACCESS_SLOT_SIZE);
                updateEquipmentAfterQuickSlotChange(i);
                return;
            }
        }
    }
    else {
        // Возвращаем в обычный слот инвентаря
        items.push_back(std::move(draggingItem));
    }
}

bool Inventory::addItemToFirstFreeSlot(std::unique_ptr<Item> item) {
    item->scaleToSize(SLOT_SIZE, SLOT_SIZE);

    // Сначала проверяем быстрые слоты
    for (int i = 0; i < 9; ++i) {
        if (!quickAccessItems[i] && canPlaceInQuickSlot(item->type)) {
            quickAccessItems[i] = std::move(item);
            quickAccessItems[i]->scaleToSize(QUICK_ACCESS_SLOT_SIZE, QUICK_ACCESS_SLOT_SIZE);
            quickAccessItems[i]->setPosition(quickAccessSlots[i].getPosition());
            return true;
        }
    }

    // Затем обычные слоты инвентаря
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            sf::Vector2f slotPos = getSlotPosition(x, y);
            bool slotFree = true;

            for (const auto& existingItem : items) {
                if (existingItem->sprite.getPosition() == slotPos) {
                    slotFree = false;
                    break;
                }
            }

            if (slotFree) {
                item->setPosition(slotPos);
                items.push_back(std::move(item));
                return true;
            }
        }
    }
    return false;
}

void Inventory::equipWeaponFromQuickSlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= 9 || !quickAccessItems[slotIndex]) return;

    // Сначала снимаем текущее оружие
    unequipCurrentWeapon();

    // Экипируем новое оружие
    WeaponItem* weaponItem = dynamic_cast<WeaponItem*>(quickAccessItems[slotIndex].get());
    if (weaponItem) {
        player.setWeapon(weaponItem->weapon);
        lastEquippedWeaponSlot = slotIndex;
        std::cout << "Equipped weapon: " << weaponItem->name << std::endl;
    }
}

void Inventory::unequipCurrentWeapon() {
    if (lastEquippedWeaponSlot != -1 &&
        quickAccessItems[lastEquippedWeaponSlot] &&
        quickAccessItems[lastEquippedWeaponSlot]->type == ItemType::WEAPON) {
        quickAccessItems[lastEquippedWeaponSlot]->unequip(player);
    }
    lastEquippedWeaponSlot = -1;
    player.unqWeapon(); // Гарантированно снимаем оружие с персонажа
}

void Inventory::useQuickAccessItem(int index) {
    if (index < 0 || index >= 9 || !quickAccessItems[index]) return;

    Item* item = quickAccessItems[index].get();

    switch (item->type) {
    case ItemType::WEAPON:
        // Для оружия просто выбираем слот (если уже выбрано - ничего не делаем)
        if (lastEquippedWeaponSlot != index) {
            equipWeaponFromQuickSlot(index);
        }
        break;

    case ItemType::CONSUMABLE:
        // Для расходуемых предметов - используем
        if (ConsumableItem* consumable = dynamic_cast<ConsumableItem*>(item)) {
            consumable->use(player);
            quickAccessItems[index].reset();
            if (lastEquippedWeaponSlot == index) {
                unequipCurrentWeapon();
            }
        }
        break;

    default:
        // Для других типов предметов просто выбираем слот
        break;
    }
}

bool Inventory::canPlaceInQuickSlot(ItemType type) const {
    return type == ItemType::WEAPON ||
        type == ItemType::CONSUMABLE ||
        (type == ItemType::HELMET && !isArmorType(type)); // Пример для других типов
}

bool Inventory::isEquipmentSlotPosition(const sf::Vector2f& pos) const {
    return pos == helmetSlot.getPosition() ||
        pos == bodySlot.getPosition() ||
        pos == legsSlot.getPosition() ||
        pos == shoesSlot.getPosition();
}

bool Inventory::isQuickSlotPosition(const sf::Vector2f& pos) const {
    for (int i = 0; i < 9; ++i) {
        if (quickAccessSlots[i].getPosition() == pos) {
            return true;
        }
    }
    return false;
}



void Inventory::updateEquipmentAfterQuickSlotChange(int slotIndex) {
    // Если выбран пустой слот - снимаем оружие
    if (!quickAccessItems[slotIndex] ||
        quickAccessItems[slotIndex]->type != ItemType::WEAPON) {
        unequipCurrentWeapon();
        lastEquippedWeaponSlot = -1;
    }
    // Если выбран слот с оружием - экипируем его
    else if (slotIndex == activeQuickSlot) {
        equipWeaponFromQuickSlot(slotIndex);
    }
}
void Inventory::updateTooltip(const sf::Vector2f& mousePos) {
    showTooltip = false;

    // Проверяем обычные слоты инвентаря
    for (auto& item : items) {
        if (item->sprite.getGlobalBounds().contains(mousePos)) {
            createTooltipContent(item.get(), mousePos);
            return;
        }
    }

    // Проверяем слоты быстрого доступа
    for (int i = 0; i < 9; ++i) {
        if (quickAccessItems[i] && quickAccessSlots[i].getGlobalBounds().contains(mousePos)) {
            createTooltipContent(quickAccessItems[i].get(), mousePos);
            return;
        }
    }

    // Проверяем экипированные предметы
    if (helmetSlot.getGlobalBounds().contains(mousePos) && equippedHelmet) {
        createTooltipContent(equippedHelmet.get(), mousePos);
        return;
    }
    if (bodySlot.getGlobalBounds().contains(mousePos) && equippedBody) {
        createTooltipContent(equippedBody.get(), mousePos);
        return;
    }
    if (legsSlot.getGlobalBounds().contains(mousePos) && equippedLegs) {
        createTooltipContent(equippedLegs.get(), mousePos);
        return;
    }
    if (shoesSlot.getGlobalBounds().contains(mousePos) && equippedShoes) {
        createTooltipContent(equippedShoes.get(), mousePos);
        return;
    }

    // Проверяем пустые слоты экипировки
    if (helmetSlot.getGlobalBounds().contains(mousePos)) {
        return;
    }
    if (bodySlot.getGlobalBounds().contains(mousePos)) {
        return;
    }
    if (legsSlot.getGlobalBounds().contains(mousePos)) {
        return;
    }
    if (shoesSlot.getGlobalBounds().contains(mousePos)) {
        return;
    }

    // Проверяем пустые слоты инвентаря
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            sf::FloatRect slotRect(getSlotPosition(x, y),
                sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
            if (slotRect.contains(mousePos)) {
                return;
            }
        }
    }

    // Проверяем пустые слоты быстрого доступа
    for (int i = 0; i < 9; ++i) {
        if (quickAccessSlots[i].getGlobalBounds().contains(mousePos)) {
            
            return;
        }
    }
}
void Inventory::createTooltipContent(Item* item, const sf::Vector2f& mousePos) {
    std::string tooltipStr;

    // Базовая информация
    tooltipStr = (item->name) + "\n";

    // Информация в зависимости от типа предмета
    switch (item->type) {
    case ItemType::CONSUMABLE: {
        auto consumable = dynamic_cast<ConsumableItem*>(item);
        if (consumable) {
            tooltipStr += "Материал\n";
            tooltipStr += "Восстанавливает " + std::to_string(static_cast<int>(consumable->healthRestore)) + " ОЗ\n";
            if (consumable->isPotion) {
                tooltipStr += "Используемый - пкм\n";
            }
        }
        break;
    }
    case ItemType::WEAPON: {
        auto weapon = dynamic_cast<WeaponItem*>(item);
        std::string str;
        if (weapon) {
            tooltipStr += "Урон: " + std::to_string(static_cast<int>(weapon->weapon.damage)) + "\n";
            if (weapon->weapon.criticalChance > 0.01)
            {
                tooltipStr += "Шанс критического урона: " + std::to_string(static_cast<int>(weapon->weapon.criticalChance * 100)) + "%\n";
            }
            
            if (weapon->weapon.attackSpeed < 100)
            {
                str = "Низкая\n";
            }
            if (weapon->weapon.attackSpeed > 100)
            {
                str = "Средняя\n";
            }
            if (weapon->weapon.attackSpeed > 150)
            {
                str = "Высокая\n";
            }
            if (weapon->weapon.attackSpeed > 200)
            {
                str = "Очень высокая\n";
            }
            if (weapon->weapon.attackSpeed > 250)
            {
                str = "Безумная\n";
            }
            tooltipStr += "Скорость атаки: " + str;
            
        }
        break;
    }
    case ItemType::HELMET:
    case ItemType::BODY:
    case ItemType::LEGS:
    case ItemType::SHOES: {
        auto armor = dynamic_cast<ArmorItem*>(item);
        if (armor) {
            tooltipStr += "Защита: " + std::to_string(static_cast<int>(armor->armor.defense)) + "\n";
        }
        break;
    }
    default:
        break;
    }

    if (item->isStackable) {
        tooltipStr += "Кол-во: " + std::to_string(item->stackSize) + "/" + std::to_string(item->maxStackSize) + "\n";
    }

    setupTooltipVisuals(tooltipStr, mousePos);
}
void Inventory::createEmptySlotTooltip(const std::string& slotName, const sf::Vector2f& mousePos) {
    std::string tooltipStr = slotName + "\n";
    setupTooltipVisuals(tooltipStr, mousePos);
}
void Inventory::setupTooltipVisuals(const std::string& text, const sf::Vector2f& mousePos) {
    tooltipText.setString(text);

    // Позиционируем tooltip рядом с курсором
    tooltipPosition = mousePos + sf::Vector2f(20.f, 20.f);
    tooltipText.setPosition(tooltipPosition);

    // Настраиваем фон tooltip
    sf::FloatRect textBounds = tooltipText.getLocalBounds();
    tooltipBackground.setSize(sf::Vector2f(textBounds.width + 10.f, textBounds.height + 10.f));
    tooltipBackground.setPosition(tooltipPosition - sf::Vector2f(5.f, 5.f));

    showTooltip = true;
}