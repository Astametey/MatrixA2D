#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include <vector>

class Player;

struct ProjectileData {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float lifetime;
    sf::Vector2f position;
    float damage;
    mutable bool hit; // Флаг, указывающий, что снаряд уже нанес урон

    ProjectileData(sf::Vector2f pos, sf::Vector2f dir, float speed, float dmg)
        : velocity(dir* speed), damage(dmg), lifetime(2.0f), hit(false) {
        shape.setSize(sf::Vector2f(8.f, 8.f)); // Размер области атаки
        shape.setFillColor(sf::Color(255, 0, 0, 150)); // Полупрозрачный красный
        shape.setOrigin(4.f, 4.f); // Центрируем
        shape.setPosition(pos);
    }

    void update(float deltaTime) {
        shape.move(velocity * deltaTime);  // Обновляем позицию shape
        lifetime -= deltaTime;
    }

    bool isAlive() const { return lifetime > 0.f && !hit; }
};
// struct armor
struct Armor {
    std::string name;
    float defense; 
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Sprite handSprite;
    sf::Vector2f offset;
    sf::Vector2f set_rect_xy;

    Armor() : name(""), defense(0.0f), offset(0, 0) {}

    bool loadTexture(const std::string& filename) {
        if (!texture.loadFromFile(filename)) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            return false;
        }
        sprite.setTexture(texture, true); // true - чтобы скопировать текстуру
        handSprite.setTexture(texture, true);
        return true;
    }

    void setPosition(const sf::Vector2f& position) {
        sprite.setPosition(position);
        handSprite.setPosition(position);
    }

    void setOffset(const sf::Vector2f& newOffset) {
        offset = newOffset;
        handSprite.setPosition(sprite.getPosition() + offset);
    }

    //иконка
    void set_sprite_icon(int x, int y, int w, int h ) {
        sprite.setTextureRect(sf::IntRect(x, y, w, h));
    }

    sf::Sprite HandGetTexture()
    {
        return handSprite;
    }


};

// struxt weapon
struct Weapon {
    enum class Type {
        MELEE,
        RANGED
    };

    std::string name;
    Type type;
    float damage;
    float attackSpeed = 200;
    float attackCooldown = 0.3f;
    float criticalChance;  // chance to critical
    float criticalMultiplier; // critical
    float pushForce = 50.0f;

    float projectileSpeed = 100.f;
    float projectileLifetime = 2.0f;

    //textures
    std::string texturePath;  // Храним путь к текстуре
    sf::IntRect textureRect;
    sf::Texture texture;
    sf::Sprite sprite;


    sf::Vector2f meleeWeaponOffset;
    sf::FloatRect collisionRect; // Прямоугольник коллизии оружия
    sf::Vector2f collisionOffset; // Смещение относительно позиции оружия
    
    void setType(int typeNum) { 
        switch (typeNum) {
        case 0: type = Type::MELEE; break;
        case 1: type = Type::RANGED; break;
        default: type = Type::MELEE; break;
        }
    }

    Weapon() :
        name(""),
        damage(1.0f),
        attackSpeed(100.0f),
        criticalChance(0.1f),
        criticalMultiplier(2.0f),
        collisionRect(0, 0, 16, 16), // Размеры по умолчанию
        collisionOffset(0, 0)
    {}

    Weapon(const std::string& path) :
        texturePath(path),
        name(""),
        damage(1.0f),
        attackSpeed(100.0f),
        criticalChance(0.1f),
        criticalMultiplier(2.0f),
        collisionRect(0, 0, 16, 16),
        collisionOffset(0, 0)
    {
        // Загружаем текстуру сразу
        if (!texture.loadFromFile(texturePath)) {
            std::cerr << "Failed to load weapon texture: " << texturePath << std::endl;
        }
    }

    void setTextureRect(int x, int y, int width, int height) {
        textureRect = sf::IntRect(x, y, width, height);
    }

    bool loadTexture(const std::string& filename) {
        texturePath = filename;

        return true;
    }

    void setPosition(const sf::Vector2f& position) {
        sprite.setPosition((position + meleeWeaponOffset));
    }

    void set_sprite_icon(int x, int y, int w, int h) {
        sprite.setTextureRect(sf::IntRect(x, y, w, h));
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

     bool flipCollision = false; // Нужно ли отражать коллизию по X
    
    // Возвращает глобальные границы коллизии с учетом направления атаки
     sf::FloatRect getCollisionBounds(bool isFacingRight) const {
         sf::FloatRect bounds = collisionRect;
         sf::Vector2f weaponPos = sprite.getPosition();

         if (isFacingRight) {
             bounds.left = weaponPos.x + collisionOffset.x ;
         }
         else {
             bounds.left = weaponPos.x - collisionOffset.x - bounds.width;
         }

         bounds.top = weaponPos.y + collisionOffset.y;
         return bounds;
     }
    
    // Установка параметров коллизии (локальные координаты)
     void setCollision(float left, float top, float width, float height, bool isFacingRight = true) {
         collisionRect = sf::FloatRect(left, top, width, height);
     }

};

class Player : public Entity {
public:
    bool isAttacking = false;
    bool ifEqupWeapon = false;
    sf::Vector2f mousePos;

    Armor armorHead;
    Armor armorBody;
    Armor armorLegs;
    Armor armorShoes;
    Weapon meleeWeapon;
    float pushForce = 100.0f;
    Weapon rangedWeapon;

    //sprites player
    sf::Sprite frontHandSprite;
    sf::Sprite backHandSprite;

    bool checkCollisionWithLevel(const Level& level);
    void resolveCollision(const sf::FloatRect& obstacle);//colision with solid

    Player() {};
    void pushBack(const sf::Vector2f& direction, float force);
	
    Player(float x, float y, float collisionWidth, float collisionHeight, float visualWidth, float visualHeight);

    ~Player() override {};
	
	void update(float deltaTime) override;

    void draw(sf::RenderWindow& window) override;

    float getDefense() const;
    float getSpeed() const;
    float getBaseDamage(bool useMeleee) const;
    float getAttackDamage(bool useMelee = true) const;
    float getCriticalChance(bool useMelee) const;
    float getCriticalMultiplier(bool useMelee) const;

    sf::FloatRect getWeaponCollisionBounds() const;

    void setArmorHead(const Armor& armor);
    void setArmorBody(const Armor& armor);
    void setArmorLegs(const Armor& armor);
    void setArmorShoes(const Armor& armor);
    void setWeapon(const Weapon& weapon);

    void unqArmorHead();
    void unqArmorBody();
    void unqArmorLegs();
    void unqArmorShoes();
    void unqWeapon();

    void startAttack(bool autoAttack = false);
    void updateAttack(float deltaTime);
    void stopAttack();

    void handleInput(float deltaTime);
    void updateAnimation();

    void updateArmorPosition();
    void updateWeaponPosition();

    bool isMeleeEquipped() const;

    float shootCooldown = 0.5f; // Задержка между выстрелами
    float currentShootCooldown = 0.f; // Текущее время задержки

    std::vector<ProjectileData> projectiles; // Вектор снарядов
    std::vector<ProjectileData>& getProjectiles() { return projectiles; }
    const std::vector<ProjectileData>& getProjectiles() const { return projectiles; } // для константных объектов

    // Добавляем новый метод
    void shoot();

    void addDamageText(int damage, bool isCritical);
    void updateDamageTexts(float deltaTime);
    void drawDamageTexts(sf::RenderWindow& window);
    void takeDamage(int damage);
private:

    // Анимационные переменные
    sf::Texture texture;
    sf::Sprite sprite;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    sf::Clock animationClock;
    bool isFacingRight;
    sf::Vector2f frontHandOffset;
    sf::Vector2f backHandOffset;
    sf::Vector2f setHair;
    sf::Vector2f setEye;
    sf::Texture textureEye;
    sf::Sprite spriteEye;
    sf::Texture textureHair;
    sf::Sprite spriteHair;

    float getAngleToMouse() const;
	float speed; // Скорость передвижения игрока

    // Поля для атаки
    
    bool isAutoAttacking = false;
    float attackAngle = 0.f; // Текущий угол атаки
    float attackSpeed = 300.f; // Скорость атаки (градусов в секунду)
    float attackCooldown = 0.3f; // Задержка между атаками
    float currentCooldown = 0.f; // Текущее время задержки
    float attackDuration = 0.2f; // Длительность атаки
    float currentAttackTime = 0.f; // Текущее время атаки
    float attackStartAngle = 120.f; // Начальный угол атаки
    float attackEndAngle = 30.f; // Конечный угол атаки
    sf::Clock autoAttackClock; // Таймер для автоатаки
    float autoAttackInterval = 1.0f; // Интервал автоатаки
    

    std::vector<DamageText> damageTexts;
};

#endif