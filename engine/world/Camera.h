#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <SFML/Graphics.hpp>

// √лобальные переменные (объ€влены как extern)
extern sf::View main_view;
extern sf::View gui_view;
extern sf::FloatRect viewBounds;

// √лобальные константы
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
extern int LEVEL_WIDTH;  // ќбъ€вление
extern int LEVEL_HEIGHT; // ќбъ€вление

// ‘ункци€ дл€ обновлени€ камеры
void setPlayerCoordinateForView(float x, float y);

#endif // _CAMERA_H_