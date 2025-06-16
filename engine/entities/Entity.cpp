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
        position.y - collisionSize.y,
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
    collisionBox.setOutlineColor(sf::Color::Green);
    collisionBox.setOutlineThickness(1.f);

    window.draw(collisionBox);
}

sf::Font DamageText::font;
bool DamageText::fontLoaded = false;

DamageText::DamageText(float x, float y, int damage, bool isCritical) : position(x, y) {
    if (!fontLoaded) {
        if (!font.loadFromFile("resources/fonts/arial.ttf")) {
            std::cerr << "Failed to load font for damage text\n";
        }
        fontLoaded = true;
    }

    text.setFont(font);
    text.setString(std::to_string(damage));
    text.setCharacterSize(isCritical ? 20 : 14);
    text.setFillColor(isCritical ? sf::Color::Red : sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(0.5f);
    text.setPosition(position);
}

void DamageText::update(float deltaTime) {
    currentTime += deltaTime;
    position.y -= speed * deltaTime;
    text.setPosition(position);

    // Плавное исчезновение
    float alpha = 255 * (1.0f - currentTime / lifetime);
    if (alpha < 0) alpha = 0;
    sf::Color color = text.getFillColor();
    color.a = static_cast<sf::Uint8>(alpha);
    text.setFillColor(color);

    sf::Color outlineColor = text.getOutlineColor();
    outlineColor.a = static_cast<sf::Uint8>(alpha);
    text.setOutlineColor(outlineColor);
}

void DamageText::draw(sf::RenderWindow& window) const {
    window.draw(text);
}

bool DamageText::isFinished() const {
    return currentTime >= lifetime;
}