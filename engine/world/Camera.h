#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <SFML/Graphics.hpp>

// Глобальные переменные (объявлены как extern)
extern sf::View main_view;
extern sf::View gui_view;
extern sf::FloatRect viewBounds;

// Глобальные константы
const unsigned int SCREEN_WIDTH = 320;
const unsigned int SCREEN_HEIGHT = 180;
const unsigned int SCALE = 4; // Масштаб для увеличения
extern int LEVEL_WIDTH;  // Объявление
extern int LEVEL_HEIGHT; // Объявление

// Функция для обновления камеры
void setPlayerCoordinateForView(float x, float y);

#endif // _CAMERA_H_