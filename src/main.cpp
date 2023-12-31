#include <iostream>

#include "game.h"



int main()
{
    // �������� ���� ��� ���������
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "MatrixA2D");
    window.setFramerateLimit(60); // ����� ������ 30

    g_init();

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

        //step
        g_step();
        

        // ������� ���� ��������� ��������� ������
        window.clear(sf::Color(90, 101, 49));


        // �������� ���...
        // window.draw(...);

        g_draw(window);
        

        // ����� ���������� �����
        window.display();
    }


    return 0;
}
