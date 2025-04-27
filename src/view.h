#ifndef _VIEW_H_
#define _VIEW_H_

#include <SFML/Graphics.hpp>
using namespace sf;
//________GLOBAL VALUE________
int LEVEL_WIDTH;
int LEVEL_HEIGHT;

// Глобальные размеры окна 
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

sf::View main_view;

void setPlayerCoordinateForView(float x, float y) {
	float tempX = x; float tempY = y;

	//if (x < 320) tempX = 320;
	//if (y < 240) tempY = 240;//верхнюю сторону
	if (y > 624) tempY = 624;//нижнюю стороню.для новой карты

	main_view.setCenter(tempX, tempY);
}

// Получаем границы view (видимой области)
sf::FloatRect viewBounds(
	main_view.getCenter().x - main_view.getSize().x / 2,
	main_view.getCenter().y - main_view.getSize().y / 2,
	main_view.getSize().x,
	main_view.getSize().y
);



#endif // _VIEW_H_