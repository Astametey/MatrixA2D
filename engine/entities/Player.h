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
    mutable bool hit; // ����, �����������, ��� ������ ��� ����� ����

    ProjectileData(sf::Vector2f pos, sf::Vector2f dir, float speed, float dmg)
        : velocity(dir* speed), damage(dmg), lifetime(2.0f), hit(false) {
        shape.setSize(sf::Vector2f(8.f, 8.f)); // ������ ������� �����
        shape.setFillColor(sf::Color(255, 0, 0, 150)); // �������������� �������
        shape.setOrigin(4.f, 4.f); // ����������
        shape.setPosition(pos);
    }

    void update(float deltaTime) {
        shape.move(velocity * deltaTime);  // ��������� ������� shape
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
        sprite.setTexture(texture, true); // true - ����� ����������� ��������
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

    //������
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
    std::string texturePath;  // ������ ���� � ��������
    sf::IntRect textureRect;
    sf::Texture texture;
    sf::Sprite sprite;


    sf::Vector2f meleeWeaponOffset;
    sf::FloatRect collisionRect; // ������������� �������� ������
    sf::Vector2f collisionOffset; // �������� ������������ ������� ������
    
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
        collisionRect(0, 0, 16, 16), // ������� �� ���������
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
        // ��������� �������� �����
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

     bool flipCollision = false; // ����� �� �������� �������� �� X
    
    // ���������� ���������� ������� �������� � ������ ����������� �����
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
    
    // ��������� ���������� �������� (��������� ����������)
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

    float shootCooldown = 0.5f; // �������� ����� ����������
    float currentShootCooldown = 0.f; // ������� ����� ��������

    std::vector<ProjectileData> projectiles; // ������ ��������
    std::vector<ProjectileData>& getProjectiles() { return projectiles; }
    const std::vector<ProjectileData>& getProjectiles() const { return projectiles; } // ��� ����������� ��������

    // ��������� ����� �����
    void shoot();

    void addDamageText(int damage, bool isCritical);
    void updateDamageTexts(float deltaTime);
    void drawDamageTexts(sf::RenderWindow& window);
    void takeDamage(int damage);
private:

    // ������������ ����������
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
	float speed; // �������� ������������ ������

    // ���� ��� �����
    
    bool isAutoAttacking = false;
    float attackAngle = 0.f; // ������� ���� �����
    float attackSpeed = 300.f; // �������� ����� (�������� � �������)
    float attackCooldown = 0.3f; // �������� ����� �������
    float currentCooldown = 0.f; // ������� ����� ��������
    float attackDuration = 0.2f; // ������������ �����
    float currentAttackTime = 0.f; // ������� ����� �����
    float attackStartAngle = 120.f; // ��������� ���� �����
    float attackEndAngle = 30.f; // �������� ���� �����
    sf::Clock autoAttackClock; // ������ ��� ���������
    float autoAttackInterval = 1.0f; // �������� ���������
    

    std::vector<DamageText> damageTexts;
};

#endif