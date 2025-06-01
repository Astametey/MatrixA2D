#ifndef ENTITY_H
#define ENTITY_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include "world/levelLoader.h"

class Entity {
public:
    // Конструктор
    Entity();
    Entity(float x, float y, float width, float height);

    // Виртуальный деструктор (важно для полиморфизма)
    virtual ~Entity();


    // Getters
    sf::Vector2f getPosition() const { return position; }
    sf::Vector2f getSize() const { return size; }
    sf::RectangleShape getShape() const { return shape; }
    sf::Vector2f getVelocity() const { return velocity; }

    // Setters
    void setPosition(const sf::Vector2f& newPosition) { position = newPosition; shape.setPosition(position); }
    void setVelocity(const sf::Vector2f& newVelocity) { velocity = newVelocity; }
    void setFillColor(const sf::Color& color) { shape.setFillColor(color); }

    virtual sf::FloatRect getGlobalBounds() const;

    // Обновление сущности (виртуальный, чтобы переопределять в наследниках)
    virtual void update(float deltaTime);

    // Отрисовка сущности (виртуальный, чтобы переопределять в наследниках, если нужно)
    virtual void draw(sf::RenderWindow& window);
    void drawDebugCollision(sf::RenderWindow& window) const;

    sf::IntRect rect;

    const sf::Vector2f& getCollisionSize() const { return collisionSize; }
    const sf::Vector2f& getVisualSize() const { return visualSize; }

protected:
    sf::Vector2f collisionSize; 
    sf::Vector2f visualSize; 

    sf::Vector2f position;
    sf::Vector2f size;
    sf::Vector2f velocity;
    sf::RectangleShape shape;

private:
    
};

#endif
