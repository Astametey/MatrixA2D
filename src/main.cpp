#include "game.h"


int main()
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "MatrixA2D");
    //init all
    resurce_init(&window);
    
    // ������ � ������ ��������� � ����
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // ��������� �����
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                
            }
        }

        // ���������� ��������� � ����������� �� �������� ��������� ����
       
            deltaTime = main_clock.restart().asSeconds();
            game_step(deltaTime);
            
        // ���������
        window.clear();

        game_draw(deltaTime, &window);

        window.display();
    }

    // ������� ��������
    resurce_delete();
    

    return 0;
}
