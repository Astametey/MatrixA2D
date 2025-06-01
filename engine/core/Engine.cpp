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
    UpdateGroundItems();
    cameraPosition = player.getPosition();
    inventory.updateEquipmentAfterQuickSlotChange(inventory.activeQuickSlot);
    SpawnPlayerAtStart();
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
        render();
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
                inventory.tryPickupItem(player.getPosition());
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

void Engine::render() {
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
    inventory.GroundItemsrender();
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

void Engine::UpdateGroundItems() {
    auto& groundItems = levelManager.GetCurrentGroundItems();
    inventory.groundItems.clear(); // Очищаем текущие предметы
    // Перемещаем предметы из levelManager в inventory
    for (auto& item : groundItems) {
        inventory.groundItems.push_back(std::move(item));
    }
    groundItems.clear(); // Очищаем исходный вектор, так как владение передано
    std::cout << "groundItems.clear()" << std::endl;
}

void Engine::initResources() {

    // Шлем
    if (!ironHelmet.loadTexture("resources/textures/items/armor/helmet.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    ironHelmet.name = "Iron Helmet";
    ironHelmet.defense = 2.0f;
    ironHelmet.set_sprite_icon(64, 0, 16, 16); 
    inventory.addItem(std::make_unique<ArmorItem>(ironHelmet, ItemType::HELMET));

    // куртка
    if (!ironShirt.loadTexture("resources/textures/items/armor/shirts.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    ironShirt.name = "Iron shirts"; 
    ironShirt.defense = 3.0f;
    ironShirt.set_sprite_icon(81, 1, 15, 14);
    inventory.addItem(std::make_unique<ArmorItem>(ironShirt, ItemType::BODY));

    // dress
    if (!Dress.loadTexture("resources/textures/items/armor/dress.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    Dress.name = "tiny dress"; 
    Dress.defense = 0.0f; 
    Dress.set_sprite_icon(81, 1, 15, 14);
    inventory.addItem(std::make_unique<ArmorItem>(Dress, ItemType::BODY));

    // штаны 
    if (!ironPants.loadTexture("resources/textures/items/armor/pants.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    ironPants.name = "Iron pants";
    ironPants.defense = 3.0f;
    ironPants.set_sprite_icon(64, 0, 16, 16);
    inventory.addItem(std::make_unique<ArmorItem>(ironPants, ItemType::LEGS));
     
    // тапки
    if (!ironShoes.loadTexture("resources/textures/items/armor/shoes.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    ironShoes.name = "Iron shoes";
    ironShoes.defense = 2.0f;
    ironShoes.set_sprite_icon(64, 0, 16, 16);
    inventory.addItem(std::make_unique<ArmorItem>(ironShoes, ItemType::SHOES));

    // sword
    if (!ironSword.loadTexture("resources/textures/items/weapon/sword.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    ironSword.name = "iron Sword";
    ironSword.damage = 5.0f;
    ironSword.set_sprite_icon(32, 0, 16, 16);
    ironSword.collisionOffset = sf::Vector2f(8, -10); // Начальное смещение
    ironSword.pushForce = 300.f;
    ironSword.setType(0);
    inventory.addItem(std::make_unique<WeaponItem>(ironSword, ItemType::WEAPON));

    // bow
    if (!ironBow.loadTexture("resources/textures/items/weapon/bow.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    ironBow.name = "iron Bow";
    ironBow.damage = 2.0f;
    ironBow.set_sprite_icon(64, 0, 16, 16);
    ironBow.pushForce = 200.f;
    ironBow.projectileSpeed = 200.f;
    ironBow.setType(1);
    inventory.addItem(std::make_unique<WeaponItem>(ironBow, ItemType::WEAPON));

}
void Engine::HandleLevelTransitions() {
    const auto& objects = levelManager.GetCurrentLevel()->getObjects();
    for (const auto& object : objects) {
        if (object.type == "level_transition" && object.rect.contains(player.getPosition())) {
            std::string targetLevel = object.name;
            
            UpdateGroundItems();
            // Сохраняем состояние игрока перед переходом

            // Переключаем уровень
            levelManager.SwitchLevel(targetLevel);

            // Восстанавливаем состояние игрока
            player.setPosition(sf::Vector2f(
                object.GetPropertyFloat("spawn_x"),
                object.GetPropertyFloat("spawn_y")
            ));

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