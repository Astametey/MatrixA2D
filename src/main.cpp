#include <iostream>
#include <SFML/Graphics.hpp> //Подключается файл, отвечающий за работу с графикой SFML;


int main()
{
    // Создание окна для рисования
    sf::RenderWindow window(sf::VideoMode(800, 600), "MatrixA3D");
    window.setFramerateLimit(60); // Лимит кадров 30


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


        // Очистка окна рисования указанным цветом
        window.clear(sf::Color::Black);


        // Рисовать тут...
        // window.draw(...);


        // Конец последнего кадра
        window.display();
    }


    return 0;
}
