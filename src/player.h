#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "entity.h"


class Player;

// ��������� ��� �������� ���������� � �����
struct Armor {
    std::string name;
    float defense;
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Vector2f offset; // ��������� �������� ��� ������� ����������������

    Armor() : name(""), defense(0.0f), offset(0, 0) {}

    bool loadTexture(const std::string& filename) {
        if (!texture.loadFromFile(filename)) {
            std::cerr << "Failed to load armor texture: " << filename << std::endl;
            return false;
        }
        sprite.setTexture(texture);
        return true;
    }

    void setPosition(const sf::Vector2f& position) {
        sprite.setPosition(position);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    void setOffset(const sf::Vector2f& newOffset) {
        offset = newOffset;
    }

    // � ��������� Armor �������� ����� set_armor_type:
    void set_armor_type(int x, int y) {
        sprite.setTextureRect(sf::IntRect(x * 16, y * 32, 16, 16));
    }


};

// ��������� ��� �������� ���������� �� ������
struct Weapon {
    enum class Type {
        MELEE,
        RANGED
    };

    std::string name;
    Type type;
    float damage;
    float attackSpeed;
    float criticalChance;  // ���� ������������ �����
    float criticalMultiplier; // ��������� ������������ �����
    sf::Texture texture;
    sf::Sprite sprite;

    Weapon() :
        name(""),
        type(Type::MELEE),
        damage(1.0f),
        attackSpeed(1.0f),
        criticalChance(0.1f),
        criticalMultiplier(2.0f)
    {}

    bool loadTexture(const std::string& filename) {
        if (!texture.loadFromFile(filename)) {
            std::cerr << "Failed to load weapon texture: " << filename << std::endl;
            return false;
        }
        sprite.setTexture(texture);
        return true;
    }

    void setPosition(const sf::Vector2f& position) {
        sprite.setPosition(position);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    virtual void attack(Player* target) {
        std::cout << "Generic weapon attack!" << std::endl;
    }

};

// ����� ������, ����������� �� Entity
class Player : public Entity
{
public:
    // ��������� ����� ����� ������
    bool isAttacking = false;
    sf::Clock attackCooldown;
    float attackDuration = 0.5f; // ������������ ����� � ��������
    sf::RectangleShape attackArea; // ��� ������������ ������� �����


    Player(float x, float y, float collisionWidth, float collisionHeight, float visualWidth, float visualHeight, Level lvl)
        : Entity(x, y, collisionWidth, collisionHeight)
    {

        visualSize = sf::Vector2f(visualWidth, visualHeight);
        shape.setSize(visualSize);
        shape.setOrigin(visualSize.x / 2, visualSize.y / 2); 

        if (!texture.loadFromFile("resources/Entities/player.png")) {
            shape.setFillColor(sf::Color::Green);
        }
        else {
            sprite.setTexture(texture);
            currentFrame = 0;
            frameWidth = texture.getSize().x / 4;
            frameHeight = texture.getSize().y;
            sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
            shape.setFillColor(sf::Color::Transparent);
        }

        // ������������� origin ������� ��������� � �����
        sprite.setOrigin(frameWidth / 2.0f, frameHeight / 2.0f);

        speed = 100.0f;
        level = lvl;

        animationClock.restart();
        isFacingRight = true;

        // ��������� ������� ����� (��� �������)
        attackArea.setSize(sf::Vector2f(16, 26));
        attackArea.setFillColor(sf::Color(0, 255, 0, 100)); // �������������� �������
        attackArea.setOrigin(attackArea.getSize().x / 2, attackArea.getSize().y / 2);

    } 

    ~Player() override {}

    float getAttackDamage() const {
        float baseDamage = 0.0f;
        float criticalMultiplier = 1.0f;
        bool usingMelee = true; // ������ ������ ������

        const Weapon& currentWeapon = usingMelee ? meleeWeapon : rangedWeapon;

        // ���������, ���� �� � ������ ������
        if (currentWeapon.texture.getSize().x > 0) {
            baseDamage = currentWeapon.damage;

            // ��������� ����������� ����
            if (static_cast<float>(rand()) / RAND_MAX < currentWeapon.criticalChance) {
                criticalMultiplier = currentWeapon.criticalMultiplier;
                // ����� �������� ���������� ������ ��� ���� �����
            }
        }
        else {
            baseDamage = 1.0f; // ������� ���� ��� ������
        }

        return baseDamage * criticalMultiplier;
    }

    bool isCriticalHit() const {
        bool usingMelee = true; // ������ ������ ������
        const Weapon& currentWeapon = usingMelee ? meleeWeapon : rangedWeapon;

        if (currentWeapon.texture.getSize().x > 0) {
            return (static_cast<float>(rand()) / RAND_MAX < currentWeapon.criticalChance);
        }
        return false;
    }

    void handleAttack()
    {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !isAttacking && attackCooldown.getElapsedTime().asSeconds() > attackDuration * 2)
        {
            isAttacking = true;
            attackCooldown.restart();

            // ����������, ����� ������ ������� (����� ����� �������� ������ ������������)
            Weapon* currentWeapon = &meleeWeapon; // �� ��������� �������

            // ���� � ������ ���� ������ �������� ��� � ��� �������
            if (rangedWeapon.texture.getSize().x > 0 && /* ������� ������ �������� ������ */ false)
            {
                currentWeapon = &rangedWeapon;
            }

            // �������� ��������������� �����
            if (currentWeapon->type == Weapon::Type::MELEE)
            {
                performMeleeAttack();
            }
            else
            {
                performRangedAttack();
            }
        }

        // ��������� ����� �� ��������� �������
        if (isAttacking && attackCooldown.getElapsedTime().asSeconds() > attackDuration)
        {
            isAttacking = false;
        }
    }

    void performMeleeAttack()
    {
        std::cout << "Performing melee attack!" << std::endl;

        // ������� ������� ����� ����� ����������
        sf::Vector2f attackPosition = position;
        float attackOffset = isFacingRight ? 8.f : -8.f;
        attackPosition.x += attackOffset;

        // ������������� ������� � ������ ������� �����
        attackArea.setPosition(attackPosition);

        // ��� ������� ������� ������� ������� �����
        std::cout << "Attack area at: " << attackPosition.x << ", " << attackPosition.y << std::endl;

        // ����������� �������� �����
        currentFrame = 0;
        sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight));

        // ���������� ������ ��������, ����� ���� ����� ��������
        animationClock.restart();
    }

    void performRangedAttack()
    {
        std::cout << "Performing ranged attack!" << std::endl;

        // ����� ������ ���� ������ �������� �������
        // ��������, �������� ����, ������� ����� � ����������� �������

        // ����������� �������� �����
        currentFrame = 3;
        sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight));
    }

    void update(float deltaTime) override {
        handleInput(deltaTime);
        handleAttack(); // ��������� ����� ��������� �����
        Entity::update(deltaTime);

        // ���������� ��������
        updateAnimation();

        // �������� ������� � ����������� �� ����������� ��������
        if (velocity.x > 0) {
            isFacingRight = true;
        }
        else if (velocity.x < 0) {
            isFacingRight = false;
        }

        // ��������� �������� �������
        float scaleX = isFacingRight ? 1.f : -1.f;
        sprite.setScale(scaleX, 1.f);

        sprite.setPosition(position);

        // ��������� ������������� ��������
        rect.left = static_cast<int>(position.x - collisionSize.x / 2);
        rect.top = static_cast<int>(position.y - collisionSize.y / 2);

        // ��������� ������� � ������� �����
        sf::Vector2f armorScale = sprite.getScale();

        armorHead.setPosition(position);
        armorBody.setPosition(position);
        armorLegs.setPosition(position);
        armorShoes.setPosition(position);

        armorHead.sprite.setScale(armorScale);
        armorBody.sprite.setScale(armorScale);
        armorLegs.sprite.setScale(armorScale);
        armorShoes.sprite.setScale(armorScale);

        meleeWeapon.sprite.setScale(armorScale);
        meleeWeapon.setPosition(position);

        rangedWeapon.sprite.setScale(armorScale);
        rangedWeapon.setPosition(position);



    }

    sf::FloatRect getAttackAreaBounds() const {
        return attackArea.getGlobalBounds();
    }

    // ��� ��������� ����� �������� ��������:
    void setArmorHead(const Armor& armor) {
        armorHead = armor;
        armorHead.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
    }
    void setArmorBody(const Armor& armor) {
        armorBody = armor;
        armorBody.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
       
    }
    void setArmorLegs(const Armor& armor) {
        armorLegs = armor;
        armorLegs.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
        
    }
    void setArmorShoes(const Armor& armor) {
        armorShoes = armor;
        armorShoes.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);

    }
    void setMeleeWeapon(const Weapon& weapon) {
        meleeWeapon = weapon;
        meleeWeapon.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);

    }
    void setRangedWeapon(const Weapon& weapon) {
        rangedWeapon = weapon;
        rangedWeapon.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);

    }

    void handleInput(float deltaTime)
    {
        velocity.x = 0;
        velocity.y = 0;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            velocity.x -= speed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            velocity.x += speed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            velocity.y -= speed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            velocity.y += speed;
        }

        // Normalize velocity if moving diagonally
        if (velocity.x != 0 && velocity.y != 0)
        {
            velocity /= std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            velocity *= speed;
        }
        armorBody.sprite.setTextureRect(sprite.getTextureRect());
        armorLegs.sprite.setTextureRect(sprite.getTextureRect());
        armorShoes.sprite.setTextureRect(sprite.getTextureRect());
        meleeWeapon.sprite.setTextureRect(sprite.getTextureRect());
    }

    // ���������� ��������
    void updateAnimation()
    {
        // ���� ���� ����� - �� ��������� �������� ��������
        if (isAttacking) return;

        // ��������� ��� �������� ��������...
        if (velocity.x != 0 || velocity.y != 0)
        {
            if (animationClock.getElapsedTime().asMilliseconds() > 100)
            {
                currentFrame = (currentFrame + 1) % 4;
                sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight));
                animationClock.restart();
            }
        }
        else
        {
            currentFrame = 1;
            sprite.setTextureRect(sf::IntRect(frameWidth, 0, frameWidth, frameHeight));
        }

        
    }

    

protected:
    float speed;
    Armor armorHead;
    Armor armorBody;
    Armor armorLegs;
    Armor armorShoes;
    Weapon meleeWeapon;
    Weapon rangedWeapon;
    Level level;

    // ������������ ����������
    sf::Texture texture;
    sf::Sprite sprite;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    sf::Clock animationClock;
    bool isFacingRight;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {

        // ��������� rect ��� �������
        sf::RectangleShape debugRect(sf::Vector2f(rect.width, rect.height));
        debugRect.setPosition(rect.left, rect.top);
        debugRect.setFillColor(sf::Color::Transparent);
        debugRect.setOutlineColor(sf::Color::Green);
        debugRect.setOutlineThickness(1);
        target.draw(debugRect, states);

        // ������ ������ ������
        if (texture.getSize().x > 0) {
            target.draw(sprite, states);
        } 
        else {
            Entity::draw(target, states);
        }


        // ������ ����������
        target.draw(armorHead.sprite, states);
        target.draw(armorBody.sprite, states);
        target.draw(armorLegs.sprite, states);
        target.draw(armorShoes.sprite, states);
        target.draw(meleeWeapon.sprite, states);
        target.draw(rangedWeapon.sprite, states);

        // ������ ������� ����� ������, ����� isAttacking = true
            if (isAttacking)
            {
                target.draw(attackArea, states);
            }

    }
};

#endif 