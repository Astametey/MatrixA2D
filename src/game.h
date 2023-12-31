#include "player.h"
#include "map.h"
#include "view.h"//���������� ��� � ����� ������


//Global init
float WINDOW_W = 1280.f;
float WINDOW_H = 1024.f;

float CurrentFrame = 0;//������ ������� ����
Player p("cat.png", 31, 31, 31.0, 31.0);//������� ������ p ������ player,������ "hero.png" ��� ��� �����+����������, ����� ���������� �,�, ������, ������.


//texture
sf::Image map_image;//������ ����������� ��� �����
sf::Texture map;//�������� �����
sf::Sprite s_map;//������ ������ ��� �����

void g_init()
{
    view.reset(sf::FloatRect(0, 0, 400, 300));//������ "����" ������ ��� �������� ������� ���� ������. (����� ����� ������ ��� �����) ��� �� ���� �������������.


    
    map_image.loadFromFile("resources/tileset.png");//��������� ���� ��� �����
    map.loadFromImage(map_image);//�������� �������� ���������
    s_map.setTexture(map);//�������� �������� ��������
}


void g_step()
{
    
    CurrentFrame += 0.05;
    
    if (CurrentFrame > 4) CurrentFrame -= 4;
    p.sprite.setTextureRect(sf::IntRect(32 * int(CurrentFrame), 32, 32, 32));
    getplayercoordinateforview(p.sprite.getPosition().x + 16, p.sprite.getPosition().y + 16);//�������� ���������� ������ � ������� ���������� �������



    p.update(CurrentFrame);
}

void g_draw(sf::RenderWindow& window)
{
    /////////////////////////////������ �����/////////////////////
    for (int i = 0; i < HEIGHT_MAP; i++)
        for (int j = 0; j < WIDTH_MAP; j++)
        {
            if (TileMap[i][j] == ' ')  s_map.setTextureRect(sf::IntRect(0, 0, 32, 32)); //���� ��������� ������ ������, �� ������ 1� ���������
            if (TileMap[i][j] == '0')  s_map.setTextureRect(sf::IntRect(32, 0, 32, 32));//���� ��������� ������ 0, �� ������ 2� ���������
            if ((TileMap[i][j] == 's')) s_map.setTextureRect(sf::IntRect(64, 0, 32, 32));//���� ��������� ������ s, �� ������ 3� ���������


            s_map.setPosition(j * 32, i * 32);//�� ���� ����������� ����������, ��������� � �����. �� ���� ������ ������� �� ��� �������. ���� ������, �� ��� ����� ���������� � ����� �������� 32*32 � �� ������ ���� �������

            window.draw(s_map);//������ ���������� �� �����
        }

    window.draw(p.sprite);//player
    window.setView(view);//"��������" ������ � ���� sfml
}