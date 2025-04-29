#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <random> // Для генерации случайных чисел

#include "level.h"
#include "pathfindingA.h"

//________GLOBAL VALUE________
sf::Vector2f GlobalmousePos;

// Базовый класс для всех игровых сущностей
class Entity : public sf::Drawable
{
public:
    Entity(float x, float y, float width, float height) : position(x, y), collisionSize(width, height), visualSize(width, height)
    {
        shape.setPosition(position);
        shape.setSize(visualSize);
        shape.setFillColor(sf::Color::Red); // Default color for entities


        rect.width = collisionSize.x;
        rect.height = collisionSize.y;
        rect.left = x - collisionSize.x / 2;
        rect.top = y - collisionSize.y / 2;

        shape.setOrigin(visualSize.x / 2, visualSize.y / 2);
    }

    virtual ~Entity() {}

    virtual void update(float deltaTime)
    {
        // Basic movement logic (can be overridden)
        position += velocity * deltaTime;
        shape.setPosition(position);
        rect.left = getPosition().x;
        rect.top = getPosition().y;
    }

    virtual sf::FloatRect getGlobalBounds() const {
        return sf::FloatRect(
            position.x - collisionSize.x / 2,
            position.y - collisionSize.y / 2,
            collisionSize.x,
            collisionSize.y
        );
    }

    // Getters
    sf::Vector2f getPosition() const { return position; }
    sf::Vector2f getSize() const { return size; }
    sf::RectangleShape getShape() const { return shape; }
    sf::Vector2f getVelocity() const { return velocity; }

    // Setters
    void setPosition(const sf::Vector2f& newPosition) { position = newPosition; shape.setPosition(position); }
    void setVelocity(const sf::Vector2f& newVelocity) { velocity = newVelocity; }
    void setFillColor(const sf::Color& color) { shape.setFillColor(color); }

    sf::IntRect rect;

    const sf::Vector2f& getCollisionSize() const { return collisionSize; }
    const sf::Vector2f& getVisualSize() const { return visualSize; }

protected:
    sf::Vector2f collisionSize; // Размер для коллизий
    sf::Vector2f visualSize;    // Размер для отображения

    sf::Vector2f position;
    sf::Vector2f size;
    sf::Vector2f velocity;
    sf::RectangleShape shape;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(shape, states);
    }
};



#endif // _ENTITY_H_