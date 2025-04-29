#include "resurce_init.h";


void cleanup() {
    
    // delete settingsMenu;
}


void game_step(float deltaT, sf::RenderWindow* win) {
    player.mousePos = GlobalmousePos;
    // Сохраняем текущую позицию
    sf::Vector2f currentPosition = player.getPosition();

    // Обновляем позицию игрока
    player.update(deltaTime);

    // Создаем прямоугольник коллизии игрока
    sf::FloatRect playerCollisionRect(
        player.getPosition().x - player.getCollisionSize().x / 2,
        player.getPosition().y - player.getCollisionSize().y / 2,
        player.getCollisionSize().x,
        player.getCollisionSize().y
    );


    // Обработка коллизий
    for (const auto& object : test_level.getObjects()) {
        if (object.type == "solid" && playerCollisionRect.intersects(object.rect)) {
            // Вычисляем перекрытие по каждой оси
            sf::FloatRect intersection;
            playerCollisionRect.intersects(object.rect, intersection);

            // Определяем, какая ось перекрыта больше
            float xOverlap = intersection.width;
            float yOverlap = intersection.height;

            // Корректируем позицию игрока, чтобы предотвратить прохождение сквозь стену
            if (xOverlap < yOverlap) {
                // Коллизия по оси X (слева или справа)
                if (player.getVelocity().x > 0) {
                    // Движение вправо, столкновение справа
                    player.setPosition(sf::Vector2f(player.getPosition().x - xOverlap, player.getPosition().y));
                }
                else {
                    // Движение влево, столкновение слева
                    player.setPosition(sf::Vector2f(player.getPosition().x + xOverlap, player.getPosition().y));
                }
            }
            else {
                // Коллизия по оси Y (сверху или снизу)
                if (player.getVelocity().y > 0) {
                    // Движение вниз, столкновение снизу
                    player.setPosition(sf::Vector2f(player.getPosition().x, player.getPosition().y - yOverlap));
                }
                else {
                    // Движение вверх, столкновение сверху
                    player.setPosition(sf::Vector2f(player.getPosition().x, player.getPosition().y + yOverlap));
                }
            }
        }
    }

    // Обновление врагов
    for (auto& enemy : enemies) {
        enemy->update(deltaT, player, enemies);

        // Проверяем столкновение только с живыми врагами
        if (!enemy->isDead() && enemy->checkPlayerCollision(player)) {
            enemy->onPlayerCollision(player);
        }

        
    }

    std::for_each(enemies.begin(), enemies.end(), [&](std::unique_ptr<Enemy>& enemy_ptr) {
        Enemy& enemy = *enemy_ptr; // Получаем ссылку на объект Enemy

        if (enemy.isDead() && enemy.isDeathAnimationFinished()) {
            respawnEnemy(enemy, &test_level); // Передаем указатель на уровень
        }
        });

    main_view.setCenter(player.getPosition());

    if (inventoryOpen) {
        win->setView(win->getDefaultView());
        drawInventory(win, playerInventory);
        drawDraggedItem(win);
        win->setView(main_view); // Возвращаем основную камеру
    }

};

void game_draw(float deltaT, sf::RenderWindow* win) {


    win->setView(main_view);  // <-- Важно применить камеру перед отрисовкой

    // Рисуем игровой мир
    test_level.Draw();
    win->draw(player);

    // Рисуем врагов
    for (auto& enemy : enemies) {
        win->draw(*enemy);
    }

    // Рисуем объекты уровня
    for (const auto& object : test_level.getObjects()) {
        sf::FloatRect objectBounds(
            static_cast<float>(object.rect.left),
            static_cast<float>(object.rect.top),
            static_cast<float>(object.rect.width),
            static_cast<float>(object.rect.height)
        );

        if (!viewBounds.intersects(objectBounds)) {
            continue;
        }

        if (object.name == "Barrel") {
            test_level.tileMap_sprite.setTextureRect(sf::IntRect(256, 0, test_level.tileWidth, test_level.tileHeight));
            test_level.tileMap_sprite.setPosition(object.rect.left, object.rect.top);
        }

        if (object.type == "solid") {
            win->draw(test_level.tileMap_sprite);
        }
    }

    if (inventoryOpen) {
        drawInventory(win, playerInventory);
    }

};

void drawInventory(sf::RenderWindow* win, Inventory& inventory) {
    // Сохраняем текущий вид
    sf::View defaultView = win->getView();

    // Устанавливаем вид для интерфейса
    win->setView(win->getDefaultView());

    // Рисуем фон инвентаря (увеличили высоту для параметров)
    sf::RectangleShape inventoryBackground(sf::Vector2f(600, 500));
    inventoryBackground.setFillColor(sf::Color(70, 70, 70, 200));
    inventoryBackground.setPosition(100, 50);
    win->draw(inventoryBackground);

    // Рисуем слоты инвентаря
    const auto& items = inventory.getItems();
    for (int y = 0; y < items.size(); ++y) {
        for (int x = 0; x < items[y].size(); ++x) {
            sf::RectangleShape slot(sf::Vector2f(50, 50));
            slot.setPosition(110 + x * 55, 130 + y * 55);
            slot.setFillColor(sf::Color(100, 100, 100));
            slot.setOutlineThickness(1);
            slot.setOutlineColor(sf::Color::White);
            win->draw(slot);

            if (!items[y][x].isEmpty()) {
                sf::Sprite itemSprite = items[y][x].item->getSprite();
                itemSprite.setPosition(115 + x * 55, 135 + y * 55);

                // Масштабируем предметы под размер ячейки
                float scaleX = 40.0f / itemSprite.getLocalBounds().width;
                float scaleY = 40.0f / itemSprite.getLocalBounds().height;
                itemSprite.setScale(scaleX, scaleY);

                win->draw(itemSprite);

                if (items[y][x].quantity > 1) {
                    sf::Text quantityText;
                    quantityText.setString(std::to_string(items[y][x].quantity));
                    quantityText.setPosition(140 + x * 55, 140 + y * 55);
                    quantityText.setCharacterSize(12);
                    quantityText.setFont(base_font);
                    quantityText.setFillColor(sf::Color::White);
                    win->draw(quantityText);
                }
            }
        }
    }

    // Рисуем слоты экипировки с подписями
    const std::vector<std::string> slotNames = { "Head", "Body", "Legs", "Shoes", "Weapont", "bow" };
    const auto& equipment = inventory.getEquipment();
    for (int i = 0; i < equipment.size(); ++i) {
        sf::RectangleShape slot(sf::Vector2f(50, 50));
        slot.setPosition(110 + i * 55, 60);
        slot.setFillColor(sf::Color(150, 150, 150));
        slot.setOutlineThickness(1);
        slot.setOutlineColor(sf::Color::Yellow);
        win->draw(slot);

        // Подпись слота
        sf::Text slotText;
        slotText.setString(slotNames[i]);
        slotText.setCharacterSize(12);
        slotText.setFont(base_font);
        slotText.setFillColor(sf::Color::White);
        slotText.setPosition(110 + i * 55, 112);
        win->draw(slotText);

        if (!equipment[i].isEmpty()) {
            sf::Sprite itemSprite = equipment[i].item->getSprite();
            itemSprite.setPosition(115 + i * 55, 65);

            // Масштабируем предметы экипировки
            float scaleX = 40.0f / itemSprite.getLocalBounds().width;
            float scaleY = 40.0f / itemSprite.getLocalBounds().height;
            itemSprite.setScale(scaleX, scaleY);

            win->draw(itemSprite);
        }
    }

    // Отображаем параметры игрока
    sf::Text statsText;
    statsText.setFont(base_font);
    statsText.setCharacterSize(16);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition(110, 410);

    std::string statsStr = "Parameters:\n";
    statsStr += "Defense: " + std::to_string(static_cast<int>(player.getDefense())) + "\n";
    statsStr += "Damage: " + std::to_string(static_cast<int>(player.getBaseDamage())) + "\n";
    statsStr += "Crit: " + std::to_string(static_cast<int>(player.getCriticalChance() * 100)) + " %";

    statsText.setString(statsStr);
    win->draw(statsText);

    // Восстанавливаем вид
    win->setView(defaultView);
}


void resource_delete()
{
    cleanup();
}