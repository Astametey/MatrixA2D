#include "game.h"


int main()
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "MatrixA2D");
    //init all
    resurce_init(&window);

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

        //game step
        game_step();
        
        // Очистка окна рисования указанным цветом
        window.clear(sf::Color::Black);


        // Рисовать тут...
        // window.draw(...);
        game_draw();

        // Конец последнего кадра
        window.display();
    }


    return 0;
}
