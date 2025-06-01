#include "Camera.h"

// Определение глобальных переменных
sf::View main_view;
sf::View gui_view;
int LEVEL_WIDTH = 0;
int LEVEL_HEIGHT = 0;

sf::FloatRect viewBounds(
    main_view.getCenter().x - main_view.getSize().x / 2,
    main_view.getCenter().y - main_view.getSize().y / 2,
    main_view.getSize().x,
    main_view.getSize().y
);

void setPlayerCoordinateForView(float x, float y) {
    float tempX = x, tempY = y;
    if (y > 624) tempY = 624;
    main_view.setCenter(tempX, tempY);
}