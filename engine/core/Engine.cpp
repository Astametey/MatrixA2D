#include "Engine.h"

Engine::Engine() : m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "MatrixA2D", sf::Style::Default),
player(32, 32, 16, 16, 16, 32), inventory(m_window, player), rng(std::random_device()())
{
    cameraVelocity = sf::Vector2f(0, 0);

    m_window.setFramerateLimit(60);
    levelManager.LoadAllLevels();


    gui_view.reset(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
    gui_view.setViewport(sf::FloatRect(0, 0, 1, 1)); // �������� ���� �����
    

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
    // ������� Player � Enemies, ����� �������� ������ ������
   
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
        main_view.setCenter(cameraPosition); // ���������� ������� ������� ������
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

        // �������� ������� ���������, ���� �� �����
        
            m_window.setView(gui_view); // ������������� gui_view ����� ����������
            inventory.handleEvent(event, gui_view);
            m_window.setView(main_view); // ���������� main_view
        
    }
}

void Engine::update(float deltaTime) {
    HandleLevelTransitions();
    
    //enemySpawnTimer += deltaTime;
    //if (enemySpawnTimer >= enemySpawnInterval) {
    //    enemySpawnTimer = 0.0f;
    //    spawnEnemies();
    //}

    // ������� ������, � ������� ����������� �������� ������
    levelManager.GetCurrentEnemies().erase(
        std::remove_if(levelManager.GetCurrentEnemies().begin(), levelManager.GetCurrentEnemies().end(),
            [](const std::unique_ptr<Enemy>& enemy) {
                return enemy->isDead() && enemy->isDeathAnimationFinished();
            }),
        levelManager.GetCurrentEnemies().end());

    // ��������� ���������� ������
    for (auto& enemy : levelManager.GetCurrentEnemies()) {
        enemy->update(deltaTime, player, levelManager.GetCurrentEnemies());
        if (!enemy->isDead() && enemy->checkPlayerCollision(player)) {
            enemy->onPlayerCollision(player);
            // ����� ����� �������� ������ ��������� ����� �������
        }
    }

    // ���������� ������
    player.update(deltaTime);
    player.checkCollisionWithLevel(*levelManager.GetCurrentLevel());
    
    
    // ������� ���������� ������ �� �������
    sf::Vector2f targetPosition = player.getPosition();
    sf::Vector2f direction = targetPosition - cameraPosition;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance > 0.1f) {
        // ����������� ����������� � �������� �� ���������� (��� ���������)
        direction /= distance;
        cameraVelocity = direction * distance * cameraSmoothness * deltaTime;
        cameraPosition += cameraVelocity;
    }

    // ��������� ������� ����
    viewBounds = sf::FloatRect(cameraPosition.x - main_view.getSize().x / 2,
        cameraPosition.y - main_view.getSize().y / 2,
        main_view.getSize().x,
        main_view.getSize().y);

    // ���������� ��������� � ������ ���������
    inventory.update(deltaTime, gui_view);
    
    
}

void Engine::render(float deltaTime) {
    m_window.clear();

    // 1. ����� ������ (����� ������ ����)
    if (auto level = levelManager.GetCurrentLevel()) {
        level->DrawTiles(viewBounds, &m_window);
    }

    // 2. �������� ��� ������� ��� ��������� � ����������� �� Y
    auto objectsToRender = levelManager.GetCurrentLevel()->GetObjectsForRendering(viewBounds, player.getPosition());

    // 3. �������� ������� � �������� � ���������� �������
    auto playerBottom = player.getPosition().y + player.getCollisionSize().y / 2;

    for (size_t i = 0; i < objectsToRender.size(); ++i) {
        auto* object = objectsToRender[i];
        float objectBottom = object->rect.top + object->rect.height;

        // ���� ������ ������ ���� ��������� ����� �������
        if (objectBottom < playerBottom) {
            levelManager.GetCurrentLevel()->DrawObject(*object, viewBounds, &m_window);
        }

        // ���� ����� ���� � ��������� �������� ����� ���������� ������
        if (i + 1 < objectsToRender.size()) { 
            auto* nextObject = objectsToRender[i + 1];
            float nextObjectBottom = nextObject->rect.top + nextObject->rect.height;

            // �������� ���� ������, ������� ��������� ����� ������� � ��������� ��������
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
            // ����� �������� ������ "���������" ��������
            float scale = 1.0f + 0.1f * std::sin(deltaTime * 6.0f);
            item->sprite.setScale(scale, scale);
        }
        else {
            item->sprite.setScale(1.0f, 1.0f);
        }

        m_window.draw(item->sprite);
    }
    // 4. �������� ������
    player.draw(m_window);

    // 5. �������� �������, ������� ������ ���� ������ ����� �������
    for (auto* object : objectsToRender) {
        float objectBottom = object->rect.top + object->rect.height;
        if (objectBottom >= playerBottom) {
            levelManager.GetCurrentLevel()->DrawObject(*object, viewBounds, &m_window);
        }
    }

    // 6. �������� ������, ������� ������ ���� ����� �������
    for (auto& enemy : levelManager.GetCurrentEnemies()) {
        float enemyBottom = enemy->getPosition().y + enemy->getCollisionSize().y / 2;
        if (enemyBottom >= playerBottom) {
            enemy->draw(m_window);
        }
    }

    // 7. ������ ����������� ���� (������ ������ �����)
    for (int i = 0; i < levelManager.GetCurrentLevel()->layers.size(); i++) {
        if (levelManager.GetCurrentLevel()->layers[i].alwaysOnTop) {
            levelManager.GetCurrentLevel()->DrawLayer(i, viewBounds, &m_window);
        }
    }

    // 8. GUI ��������
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

            // ����������� �������
            levelManager.SwitchLevel(targetLevel);

            // ��������������� ������� ������
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
    // ����� �������� ������� ������ ���������, ��������� � �������
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