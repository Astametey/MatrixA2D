#include "game.h"


int main()
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "MatrixA2D");

    // Инициализация с правильным именем функции
    resource_init(&window);

    while (window.isOpen()) {
        sf::Event event;

        // Всегда обновляем позицию мыши
        GlobalmousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    // Обработка паузы
                    
                }
                else if (event.key.code == sf::Keyboard::I) {
                    inventoryOpen = !inventoryOpen;
                }
            }

        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::I) {
            inventoryOpen = !inventoryOpen;
            if (!inventoryOpen && isDragging) {
                stopDragging(playerInventory);
            }
        }

        // Обработка ввода для игрока (если инвентарь закрыт)
        if (!inventoryOpen) {
            
        }
        // Обработка инвентаря
        else {
            handleInventoryMouseEvents(event, &window, playerInventory);
        }

        // Обновление игры только если не на паузе
        
            deltaTime = main_clock.restart().asSeconds();
            game_step(deltaTime, &window);

        window.clear();
        game_draw(deltaTime, &window);

        window.display();
    }

    // Очистка ресурсов
    resource_delete();

    return 0;
}
