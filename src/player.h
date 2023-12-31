#include <SFML/Graphics.hpp>

class Player { // ����� ������
public:
    float x, y, w, h, dx, dy, speed = 0, spd = 2; //���������� ������ � � �, ������ ������, ��������� (�� � � �� �), ���� ��������
    int dir = 0; //����������� (direction) �������� ������
    sf::String File; //���� � �����������
    sf::Image image;//���� �����������
    sf::Texture texture;//���� �������� 
    sf::Sprite sprite;//���� ������

    Player(sf::String F, float X, float Y, float W, float H) {  //����������� � �����������(�����������) ��� ������ Player. ��� �������� ������� ������ �� ����� �������� ��� �����, ���������� � � �, ������ � ������
        File = F;//��� �����+����������
        w = W; h = H;//������ � ������
        image.loadFromFile("resources/" + File);//���������� � image ���� ����������� ������ File �� ��������� ��, ��� �������� ��� �������� �������. � ����� ������ "hero.png" � ��������� ������ ���������� 	image.loadFromFile("images/hero/png");
        //image.createMaskFromColor(sf::Color(41, 33, 59));//������� �������� �����-����� ����, ��� ���� ��� ���������� �� ��������.
        texture.loadFromImage(image);//���������� ���� ����������� � ��������
        sprite.setTexture(texture);//�������� ������ ���������
        x = X; y = Y;//���������� ��������� �������
        sprite.setTextureRect(sf::IntRect(0, 0, w, h));  //������ ������� ���� ������������� ��� ������ ������ ����, � �� ���� ����� �����. IntRect - ���������� �����
        sprite.setOrigin(16, 16);
    }



    void update(float CurrentFrame) //������� "���������" ������� ������. update - ����������. ��������� � ���� ����� SFML , ���������� ���� �������� ����������, ����� ��������� ��������.
    {
        switch (dir)//��������� ��������� � ����������� �� �����������. (������ ����� ������������� �����������)
        {
        case 0: dx = speed; dy = 0;   break;//�� ���� ������ ������������� ��������, �� ������ ��������. ��������, ��� �������� ���� ������ ������
        case 1: dx = -speed; dy = 0;   break;//�� ���� ������ ������������� ��������, �� ������ ��������. ����������, ��� �������� ���� ������ �����
        case 2: dx = 0; dy = speed;   break;//�� ���� ������ ������� ��������, �� ������ �������������. ����������, ��� �������� ���� ������ ����
        case 3: dx = 0; dy = -speed;   break;//�� ���� ������ ������� ��������, �� ������ �������������. ����������, ��� �������� ���� ������ �����
        }

        x += dx;//�� �������� �� �������� �����. ���� ��������� �� ����� �������� �������� ��������� � ��� ��������� ��������
        y += dy;//���������� �� ������

        speed = 0;//�������� ��������, ����� �������� �����������.
        sprite.setPosition(x, y); //������� ������ � ������� x y , ����������. ���������� ������� � ���� �������, ����� �� ��� ������ ����� �� �����.

        ///////////////////////////////////////////���������� ���������� � ���������////////////////////////////////////////////////////////////////////////
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::A))) {
            dir = 1; speed = spd;//dir =1 - ����������� �����, speed =1 - �������� ��������. �������� - ����� �� ��� ����� �� �� ��� �� �������� � ����� �� ���������� ������ ���
            sprite.setTextureRect(sf::IntRect(32 * int(CurrentFrame), 0, 32, 32)); //����� ������ p ������ player ������ ������, ����� �������� (��������� �������� �����)
            sprite.setScale(-1, 1); //������� �� �����������
        }

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::D))) {
            dir = 0; speed = spd;//����������� ������, �� ����

            sprite.setTextureRect(sf::IntRect(32 * int(CurrentFrame), 0, 32, 32)); //����� ������ p ������ player ������ ������, ����� �������� (��������� �������� �����)
            sprite.setScale(1, 1); //������� �� �����������
        }

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::W))) {
            dir = 3; speed = spd;//����������� ����, �� ����
            sprite.setTextureRect(sf::IntRect(32 * int(CurrentFrame), 0, 32, 32)); //����� ������ p ������ player ������ ������, ����� �������� (��������� �������� �����)

        }

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::S))) { //���� ������ ������� ������� ����� ��� ���� ����� �
            dir = 2; speed = spd;//����������� �����, �� ����
            sprite.setTextureRect(sf::IntRect(32 * int(CurrentFrame), 0, 32, 32)); //����� ������ p ������ player ������ ������, ����� �������� (��������� �������� �����)

        }

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && speed != 0)) {
            speed *= 2;
            sprite.setTextureRect(sf::IntRect(32 * int(CurrentFrame), 64, 32, 32));
        }
    }
};