#include "game.h"


int main()
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "MatrixA2D");
    //init all
    resurce_init(&window);

    // ������ � ������ ��������� � ����
    while (window.isOpen())
    {
        // ������� ����
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //game step
        game_step();
        
        // ������� ���� ��������� ��������� ������
        window.clear(sf::Color::Black);


        // �������� ���...
        // window.draw(...);
        game_draw();

        // ����� ���������� �����
        window.display();
    }


    return 0;
}
