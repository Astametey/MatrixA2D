#include "game.h"


int main()
{
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "MatrixA2D");
    //init all
    resurce_init(&window);
    
    // Запуск и работа программы в окне
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Обработка паузы
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                
            }
        }

        // Обновление состояния в зависимости от текущего состояния игры
       
            deltaTime = main_clock.restart().asSeconds();
            game_step(deltaTime);
            
        // Отрисовка
        window.clear();

        game_draw(deltaTime, &window);

        window.display();
    }

    // Очистка ресурсов
    resurce_delete();
    

    return 0;
}
