#include "Entity.h"

Entity::Entity(float x, float y, float width, float height) : position(x, y), collisionSize(width, height), visualSize(width, height)
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


Entity::~Entity() {
    // Здесь можно добавить код для освобождения ресурсов, если необходимо
}

void Entity::update(float deltaTime) {
    // Basic movement logic (can be overridden)
    position += velocity * deltaTime;
    shape.setPosition(position);
    rect.left = getPosition().x;
    rect.top = getPosition().y;
}

sf::FloatRect Entity::getGlobalBounds() const {
    return sf::FloatRect(
        position.x - collisionSize.x / 2,
        position.y - collisionSize.y / 2,
        collisionSize.x,
        collisionSize.y
    );
}


void Entity::draw(sf::RenderWindow& window) {
    window.draw(shape);
}



void Entity::drawDebugCollision(sf::RenderWindow& window) const {
    sf::RectangleShape collisionBox;
    collisionBox.setSize(collisionSize);
    collisionBox.setOrigin(collisionSize.x / 2, collisionSize.y / 2);
    collisionBox.setPosition(position);
    collisionBox.setFillColor(sf::Color::Transparent);
    collisionBox.setOutlineColor(sf::Color::Green); // Зеленый контур для коллизии
    collisionBox.setOutlineThickness(1.f);

    window.draw(collisionBox);
}
