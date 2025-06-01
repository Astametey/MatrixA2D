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
    UpdateGroundItems();
    cameraPosition = player.getPosition();
    inventory.updateEquipmentAfterQuickSlotChange(inventory.activeQuickSlot);
    SpawnPlayerAtStart();
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

void Engine::render() {
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
    inventory.GroundItemsrender();
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

void Engine::UpdateGroundItems() {
    auto& groundItems = levelManager.GetCurrentGroundItems();
    inventory.groundItems.clear(); // ������� ������� ��������
    // ���������� �������� �� levelManager � inventory
    for (auto& item : groundItems) {
        inventory.groundItems.push_back(std::move(item));
    }
    groundItems.clear(); // ������� �������� ������, ��� ��� �������� ��������
    std::cout << "groundItems.clear()" << std::endl;
}

void Engine::initResources() {

    // ����
    if (!ironHelmet.loadTexture("resources/textures/items/armor/helmet.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    ironHelmet.name = "Iron Helmet";
    ironHelmet.defense = 2.0f;
    ironHelmet.set_sprite_icon(64, 0, 16, 16); 
    inventory.addItem(std::make_unique<ArmorItem>(ironHelmet, ItemType::HELMET));

    // ������
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

    // ����� 
    if (!ironPants.loadTexture("resources/textures/items/armor/pants.png")) {
        std::cerr << "Failed to load helmet texture!" << std::endl;
    }
    ironPants.name = "Iron pants";
    ironPants.defense = 3.0f;
    ironPants.set_sprite_icon(64, 0, 16, 16);
    inventory.addItem(std::make_unique<ArmorItem>(ironPants, ItemType::LEGS));
     
    // �����
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
    ironSword.collisionOffset = sf::Vector2f(8, -10); // ��������� ��������
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
            // ��������� ��������� ������ ����� ���������

            // ����������� �������
            levelManager.SwitchLevel(targetLevel);

            // ��������������� ��������� ������
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