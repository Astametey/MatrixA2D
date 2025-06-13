#include "Engine.h"

Engine::Engine() : m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "MatrixA2D", sf::Style::Default),
player(32, 32, 16, 16, 16, 32), inventory(m_window, player), rng(std::random_device()())
{
    cameraVelocity = sf::Vector2f(0, 0);

    m_window.setFramerateLimit(60);
    levelManager.LoadAllLevels();


    gui_view.reset(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
    gui_view.setViewport(sf::FloatRect(0, 0, 1, 1)); // Занимает весь экран
    

    //init resources
    // Add some items
    initResources();
    inventory.updateEquipmentAfterQuickSlotChange(inventory.activeQuickSlot);
    SpawnPlayerAtStart();
    cameraPosition = player.getPosition();
    main_view.reset(sf::FloatRect(player.getPosition().x, player.getPosition().y, 640, 380));
    m_window.setView(main_view); // view windows
}

Engine::~Engine() {
    // Удаляем Player и Enemies, чтобы избежать утечек памяти
   
}


void Engine::run() {
    sf::Clock clock;

    while (m_window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        mousePos = sf::Mouse::getPosition(m_window);
        worldPos = m_window.mapPixelToCoords(mousePos);
        player.mousePos = worldPos;

        handleEvents(deltaTime);
        update(deltaTime);
        main_view.setCenter(cameraPosition); // Используем плавную позицию камеры
        render(deltaTime);
    }
}

void Engine::handleEvents(float deltaTime) {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Tab) {
                inventory.toggleVisibility();
            }
            if (event.key.code == sf::Keyboard::E) {
                inventory.tryPickupItem(player.getPosition(), levelManager.GetCurrentGroundItems());
            }
        }

        // Передаем события инвентарю, если он видим
        
            m_window.setView(gui_view); // Устанавливаем gui_view перед обработкой
            inventory.handleEvent(event, gui_view);
            m_window.setView(main_view); // Возвращаем main_view
        
    }
}

void Engine::update(float deltaTime) {
    HandleLevelTransitions();
    
    //enemySpawnTimer += deltaTime;
    //if (enemySpawnTimer >= enemySpawnInterval) {
    //    enemySpawnTimer = 0.0f;
    //    spawnEnemies();
    //}

    // Удаляем врагов, у которых завершилась анимация смерти
    levelManager.GetCurrentEnemies().erase(
        std::remove_if(levelManager.GetCurrentEnemies().begin(), levelManager.GetCurrentEnemies().end(),
            [](const std::unique_ptr<Enemy>& enemy) {
                return enemy->isDead() && enemy->isDeathAnimationFinished();
            }),
        levelManager.GetCurrentEnemies().end());

    // Обновляем оставшихся врагов
    for (auto& enemy : levelManager.GetCurrentEnemies()) {
        enemy->update(deltaTime, player, levelManager.GetCurrentEnemies());
        if (!enemy->isDead() && enemy->checkPlayerCollision(player)) {
            enemy->onPlayerCollision(player);
            // Здесь можно добавить логику получения урона игроком
        }
    }

    // Обновление игрока
    player.update(deltaTime);
    player.checkCollisionWithLevel(*levelManager.GetCurrentLevel());
    
    
    // Плавное следование камеры за игроком
    sf::Vector2f targetPosition = player.getPosition();
    sf::Vector2f direction = targetPosition - cameraPosition;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance > 0.1f) {
        // Нормализуем направление и умножаем на расстояние (для плавности)
        direction /= distance;
        cameraVelocity = direction * distance * cameraSmoothness * deltaTime;
        cameraPosition += cameraVelocity;
    }

    // Обновляем границы вида
    viewBounds = sf::FloatRect(cameraPosition.x - main_view.getSize().x / 2,
        cameraPosition.y - main_view.getSize().y / 2,
        main_view.getSize().x,
        main_view.getSize().y);

    // Обновление инвентаря и других элементов
    inventory.update(deltaTime, gui_view);
    
    
}

void Engine::render(float deltaTime) {
    m_window.clear();

    // 1. Тайлы уровня (самый нижний слой)
    if (auto level = levelManager.GetCurrentLevel()) {
        level->DrawTiles(viewBounds, &m_window);
    }

    // 2. Получаем все объекты для отрисовки с сортировкой по Y
    auto objectsToRender = levelManager.GetCurrentLevel()->GetObjectsForRendering(viewBounds, player.getPosition());

    // 3. Рендерим объекты и сущности в правильном порядке
    auto playerBottom = player.getPosition().y + player.getCollisionSize().y / 2;

    for (size_t i = 0; i < objectsToRender.size(); ++i) {
        auto* object = objectsToRender[i];
        float objectBottom = object->rect.top + object->rect.height;

        // Если объект должен быть отрисован перед игроком
        if (objectBottom < playerBottom) {
            levelManager.GetCurrentLevel()->DrawObject(*object, viewBounds, &m_window);
        }

        // Если между этим и следующим объектом нужно нарисовать врагов
        if (i + 1 < objectsToRender.size()) { 
            auto* nextObject = objectsToRender[i + 1];
            float nextObjectBottom = nextObject->rect.top + nextObject->rect.height;

            // Рендерим всех врагов, которые находятся между текущим и следующим объектом
            for (auto& enemy : levelManager.GetCurrentEnemies()) {
                float enemyBottom = enemy->getPosition().y + enemy->getCollisionSize().y / 2;
                if (enemyBottom > objectBottom && enemyBottom <= nextObjectBottom) {
                    enemy->draw(m_window);
                }
            }
        }
    }
    for (auto& item : levelManager.GetCurrentGroundItems()) {
        float distance = std::sqrt(
            std::pow(player.getPosition().x - item->sprite.getPosition().x, 2) +
            std::pow(player.getPosition().y - item->sprite.getPosition().y, 2));

        if (distance > 32) {
            // Можно добавить эффект "пульсации" предмета
            float scale = 1.0f + 0.1f * std::sin(deltaTime * 6.0f);
            item->sprite.setScale(scale, scale);
        }
        else {
            item->sprite.setScale(1.0f, 1.0f);
        }

        m_window.draw(item->sprite);
    }
    // 4. Рендерим игрока
    player.draw(m_window);

    // 5. Рендерим объекты, которые должны быть всегда перед игроком
    for (auto* object : objectsToRender) {
        float objectBottom = object->rect.top + object->rect.height;
        if (objectBottom >= playerBottom) {
            levelManager.GetCurrentLevel()->DrawObject(*object, viewBounds, &m_window);
        }
    }

    // 6. Рендерим врагов, которые должны быть перед игроком
    for (auto& enemy : levelManager.GetCurrentEnemies()) {
        float enemyBottom = enemy->getPosition().y + enemy->getCollisionSize().y / 2;
        if (enemyBottom >= playerBottom) {
            enemy->draw(m_window);
        }
    }

    // 7. Рисуем специальные слои (всегда поверх всего)
    for (int i = 0; i < levelManager.GetCurrentLevel()->layers.size(); i++) {
        if (levelManager.GetCurrentLevel()->layers[i].alwaysOnTop) {
            levelManager.GetCurrentLevel()->DrawLayer(i, viewBounds, &m_window);
        }
    }

    // 8. GUI элементы
    m_window.setView(gui_view);
    inventory.renderUI();
    if (inventory.isVisible()) {
        inventory.render();
    }
    m_window.setView(main_view);

    m_window.display();
}


void Engine::initResources() {

    auto sword = levelManager.CreateItemFromTemplate("steel_sword", { 0,0 });
    if (sword) {
        inventory.addItem(std::move(sword));
    }
}
void Engine::HandleLevelTransitions() {
    const auto& objects = levelManager.GetCurrentLevel()->getObjects();
    for (const auto& object : objects) {
        if (object.type == "level_transition" && object.rect.contains(player.getPosition())) {
            std::string targetLevel = object.name;

            // Переключаем уровень
            levelManager.SwitchLevel(targetLevel);

            // Восстанавливаем позицию игрока
            player.setPosition(sf::Vector2f(
                object.GetPropertyFloat("spawn_x"),
                object.GetPropertyFloat("spawn_y")
            ));
            cameraPosition = player.getPosition();

            break;
        }
    }
}


void Engine::ClearAllEntities() {
    levelManager.GetCurrentEnemies().clear();
    //levelObjects.clear();
    // Можно добавить очистку других сущностей, связанных с уровнем
}
void Engine::SpawnPlayerAtStart() {
    const auto& objects = levelManager.GetCurrentLevel()->getObjects();

    for (const auto& object : objects) {
        if (object.type == "player_spawn") {
            player.setPosition(sf::Vector2f(
                object.rect.left + object.rect.width / 2,
                object.rect.top + object.rect.height / 2
            ));
            break;
        }
    }
}