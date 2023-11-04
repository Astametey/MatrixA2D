#include "player.h"
#include "map.h"
#include "view.h"//подключили код с видом камеры


//Global init
float WINDOW_W = 1280.f;
float WINDOW_H = 1024.f;

float CurrentFrame = 0;//хранит текущий кадр
Player p("cat.png", 31, 31, 31.0, 31.0);//создаем объект p класса player,задаем "hero.png" как имя файла+расширение, далее координата Х,У, ширина, высота.


//texture
sf::Image map_image;//объект изображения для карты
sf::Texture map;//текстура карты
sf::Sprite s_map;//создаём спрайт для карты

void g_init()
{
    view.reset(sf::FloatRect(0, 0, 400, 300));//размер "вида" камеры при создании объекта вида камеры. (потом можем менять как хотим) Что то типа инициализации.


    
    map_image.loadFromFile("resources/tileset.png");//загружаем файл для карты
    map.loadFromImage(map_image);//заряжаем текстуру картинкой
    s_map.setTexture(map);//заливаем текстуру спрайтом
}


void g_step()
{
    
    CurrentFrame += 0.05;
    
    if (CurrentFrame > 4) CurrentFrame -= 4;
    p.sprite.setTextureRect(sf::IntRect(32 * int(CurrentFrame), 32, 32, 32));
    getplayercoordinateforview(p.sprite.getPosition().x + 16, p.sprite.getPosition().y + 16);//передаем координаты игрока в функцию управления камерой



    p.update(CurrentFrame);
}

void g_draw(sf::RenderWindow& window)
{
    /////////////////////////////Рисуем карту/////////////////////
    for (int i = 0; i < HEIGHT_MAP; i++)
        for (int j = 0; j < WIDTH_MAP; j++)
        {
            if (TileMap[i][j] == ' ')  s_map.setTextureRect(sf::IntRect(0, 0, 32, 32)); //если встретили символ пробел, то рисуем 1й квадратик
            if (TileMap[i][j] == '0')  s_map.setTextureRect(sf::IntRect(32, 0, 32, 32));//если встретили символ 0, то рисуем 2й квадратик
            if ((TileMap[i][j] == 's')) s_map.setTextureRect(sf::IntRect(64, 0, 32, 32));//если встретили символ s, то рисуем 3й квадратик


            s_map.setPosition(j * 32, i * 32);//по сути раскидывает квадратики, превращая в карту. то есть задает каждому из них позицию. если убрать, то вся карта нарисуется в одном квадрате 32*32 и мы увидим один квадрат

            window.draw(s_map);//рисуем квадратики на экран
        }

    window.draw(p.sprite);//player
    window.setView(view);//"оживляем" камеру в окне sfml
}