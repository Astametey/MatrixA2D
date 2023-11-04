#include <iostream>

#include "game.h"



int main()
{
    // Создание окна для рисования
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "MatrixA2D");
    window.setFramerateLimit(60); // Лимит кадров 30

    g_init();

    // Запуск и работа программы в окне
    while (window.isOpen())
    {
        // События окна
        sf::Event event;
        while (window.pollEvent(event)) 
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //step
        g_step();
        

        // Очистка окна рисования указанным цветом
        window.clear(sf::Color(90, 101, 49));


        // Рисовать тут...
        // window.draw(...);

        g_draw(window);
        

        // Конец последнего кадра
        window.display();
    }


    return 0;
}
