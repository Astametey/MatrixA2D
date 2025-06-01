#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <SFML/Graphics.hpp>

// ���������� ���������� (��������� ��� extern)
extern sf::View main_view;
extern sf::View gui_view;
extern sf::FloatRect viewBounds;

// ���������� ���������
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
extern int LEVEL_WIDTH;  // ����������
extern int LEVEL_HEIGHT; // ����������

// ������� ��� ���������� ������
void setPlayerCoordinateForView(float x, float y);

#endif // _CAMERA_H_