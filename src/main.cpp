#include "game.h"


int main()
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "MatrixA2D");

    // ������������� � ���������� ������ �������
    resource_init(&window);

    while (window.isOpen()) {
        sf::Event event;

        // ������ ��������� ������� ����
        GlobalmousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    // ��������� �����
                    
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

        // ��������� ����� ��� ������ (���� ��������� ������)
        if (!inventoryOpen) {
            
        }
        // ��������� ���������
        else {
            handleInventoryMouseEvents(event, &window, playerInventory);
        }

        // ���������� ���� ������ ���� �� �� �����
        
            deltaTime = main_clock.restart().asSeconds();
            game_step(deltaTime, &window);

        window.clear();
        game_draw(deltaTime, &window);

        window.display();
    }

    // ������� ��������
    resource_delete();

    return 0;
}
