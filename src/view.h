#ifndef _VIEW_H_
#define _VIEW_H_

#include <SFML/Graphics.hpp>
using namespace sf;
//________GLOBAL VALUE________
int LEVEL_WIDTH;
int LEVEL_HEIGHT;

// Глобальные размеры окна 
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

sf::View view;

void setPlayerCoordinateForView(float x, float y) {
	float tempX = x; float tempY = y;

	//if (x < 320) tempX = 320;
	//if (y < 240) tempY = 240;//верхнюю сторону
	if (y > 624) tempY = 624;//нижнюю стороню.для новой карты

	view.setCenter(tempX, tempY);
}


#endif // _VIEW_H_