#include "view.h"
#include "level.h"
#include "player.h"
#include "enemies.h"

//________GLOBAL VALUE________


Level test_level;

//--Entity
std::vector<std::unique_ptr<Enemy>> enemies;

//--Player
Player player(32, 32,
    16, 16,  // collision width/height (меньшие значения)
    16, 32,  // visual width/height (размер текстуры)
    test_level);
Armor head;
Armor body;
Armor legs;
Armor shoes;
Weapon sword;


//----Timers
sf::Clock main_clock;
float deltaTime;

void cleanup() {
    
    // delete settingsMenu;
}


//Инициализация ресурсов
void resurce_init(sf::RenderWindow* l_win)
{
    // Инициализация меню
    


	test_level.LoadFromFile("resources//levels//fores_level_0.tmx", "resources//levels//forest_tile_set.png", l_win);
	main_view.reset(sf::FloatRect(0, 0, 640, 360));
	//Player
	// Load armor textures
    head.loadTexture("resources/Entities/helmet.png"); // Replace with your texture path
    head.set_armor_type(2, 0);
	player.setArmorHead(head);
     
    body.loadTexture("resources/Entities/shirts.png"); // Replace with your texture path
	player.setArmorBody(body);

    legs.loadTexture("resources/Entities/pants.png"); // Replace with your texture path
    player.setArmorLegs(legs);

    shoes.loadTexture("resources/Entities/shoes.png"); // Replace with your texture path
    player.setArmorShoes(shoes);



	// Load weapon textures
	sword.loadTexture("resources/Entities/sword.png");
	player.setMeleeWeapon(sword);
    sword.damage = 2;

	// Очищаем вектор, если он уже был заполнен
	enemies.clear();

	// Добавляем врагов с разными координатами
	enemies.push_back(std::make_unique<Enemy>(100.0f, 400.0f, 32.0f, 32.0f, test_level,"resources/Entities/Orc-Sheet.png"));  // Враг 1
    enemies.push_back(std::make_unique<Enemy>(700.0f, 150.0f, 32.0f, 32.0f, test_level,"resources/Entities/Orc-Sheet.png"));  // Враг 2
    enemies.push_back(std::make_unique<Enemy>(400.0f, 300.0f, 38.0f, 33.0f, test_level, "resources/Entities/Orc-Sheet.png"));  // Враг 3
	
	// Добавьте столько врагов, сколько нужно, с разными координатами
        if (!enemies.empty()) {
            Enemy* enemy = enemies[0].get(); // Получаем сырой указатель на объект Enemy
            // Теперь мы можем использовать enemy для вызова методов:
            enemy = enemies[0].get();
            enemy->setCollisionSize(16, 16);
            enemy->setMaxHealth(10);
            enemy = enemies[1].get();
            enemy->setCollisionSize(16, 16);
            enemy->setMaxHealth(10);
            enemy = enemies[2].get();
            enemy->setCollisionSize(16, 16);
            enemy->setMaxHealth(10);
        }
        else {
            std::cout << "Вектор enemies пуст!" << std::endl;
        }

}


void game_step(float deltaT) {

    const sf::Vector2f currentPosition = player.getPosition();
    player.update(deltaT);

    // Проверка столкновений игрока
    sf::FloatRect playerCollisionRect(
        player.getPosition().x - player.getCollisionSize().x / 2,
        player.getPosition().y - player.getCollisionSize().y / 2,
        player.getCollisionSize().x,
        player.getCollisionSize().y
    );

    bool collision = false;
    for (const auto& object : test_level.getObjects()) {
        if (object.type == "solid" && playerCollisionRect.intersects(object.rect)) {
            collision = true;
            break;
        }
    }

    if (collision) {
        player.setPosition(currentPosition);
    }

    // Обновление врагов
    for (auto& enemy : enemies) {
        enemy->update(deltaT, player, enemies);

        // Проверяем столкновение только с живыми врагами
        if (!enemy->isDead() && enemy->checkPlayerCollision(player)) {
            enemy->onPlayerCollision(player);
        }
    }

    main_view.setCenter(player.getPosition());

    // Удаляем мёртвых врагов только после завершения анимации смерти
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const std::unique_ptr<Enemy>& enemy) {
                // Удаляем врага, только если он мертв и анимация смерти завершена
                return enemy->isDead() && enemy->isDeathAnimationFinished();
            }),
        enemies.end()
    );

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

};

void resurce_delete()
{
    cleanup();
}