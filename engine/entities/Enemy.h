#ifndef _ENEMIES_H_
#define _ENEMIES_H_

#include "entity.h"
#include "player.h"
#include "world/pathfindingA.h"

// Класс врага, наследуется от Entity
class Enemy : public Entity {
    // Запрещаем копирование
    Enemy(const Enemy&) = delete;
    Enemy& operator=(const Enemy&) = delete;
public:
    int framesPerAnimation; // Количество кадров на одну анимацию
    int maxHealth = 3;
    int getHealth() const { return health; }
    bool isDead() const { return health <= 0; }
    bool shouldDraw(const sf::FloatRect& viewBounds) const;

    enum class AnimationType {
        Idle,       // Покой
        Walk,       // Ходьба 
        Attack,     // Атака 
        PowerAttack,// Усиленная Атака
        Hit,        // Получение урона
        Death,      // Смерть
        Bounce      // Отталкивание
    };

    Enemy(float x, float y, float width, float height, Level& lvl,
        const std::string& texturePath = "resources/textures/entities/enemies/cat.png");

    // Разрешаем только перемещение (опционально)
    Enemy(Enemy&&) = default;
    Enemy& operator=(Enemy&&) = default;

    ~Enemy() override {}

    void Set_type_enemy(std::string type);
    void takeDamage(int damage);
    void applyPushForce(const sf::Vector2f& force);
    void update(float deltaTime, Player& player, const std::vector<std::unique_ptr<Enemy>>& allEnemies);
    sf::FloatRect getGlobalBounds() const override;
    void setHealth(int newHealth);
    void setMaxHealth(int newMaxHealth);
    void setState(AnimationType newState);
    void setAnimation(AnimationType type);
    void setCollisionSize(float width, float height);
    void setCollisionSize(const sf::Vector2f& newSize);
    bool isDeathAnimationFinished() const;
    void updateAnimation();

    // Методы для обработки состояний
    void handleIdleState(float deltaTime, const Player& player);
    void handleChaseState(float deltaTime, const Player& player);
    void handleBounceState(float deltaTime);
    void handleAttackState(float deltaTime, const Player& player);

    // Проверка столкновений
    bool checkPlayerCollision(const Player& player) const;
    bool checkCollisionWithSolids(const sf::Vector2f& newPos) const;
    void onPlayerCollision(const Player& player);
    void drawPath(sf::RenderTarget& target, sf::RenderStates states) const;
    bool isPositionWalkable(const sf::Vector2f& pos) const;
    void pushOtherEnemies(const std::vector<std::unique_ptr<Enemy>>& allEnemies);
    virtual void draw(sf::RenderWindow& window) override;
private:
    int health = 3;
    bool deathAnimationPlayed = false;
    bool deathAnimationFinished = false;
    float attackDistance = 14.0f;
    float stopDistance = 18.0f;
    bool wasHitThisAttack = false;
    float knockbackSlowdown = 0.5f;
    float bounceDuration = 0.3f;
    float currentBounceTime = 0.0f;
    sf::Vector2f bounceVelocity;
    float attackCooldown;
    sf::Vector2f attackAreaSize;

    static constexpr float PUSH_FORCE = 50.0f;
    static constexpr float MIN_PUSH_DISTANCE = 16.0f;
    static constexpr float WALL_CHECK_DISTANCE = 8.0f;

    AnimationType currentState;
    float stateTimer;
    float bounceTime;
    sf::Texture texture;
    sf::Sprite sprite;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    AnimationType currentAnimation;
    sf::Clock animationClock;
    float animationSpeed;
    bool isFacingRight;
    sf::Vector2f lastPlayerPosition;
    float pathUpdateDistance = 64.0f;

    float calculateDistance(const sf::Vector2f& pos1, const sf::Vector2f& pos2) const;
    bool isPathObstructed(const Player& player) const;
    void calculatePath(const Player& player);
    void moveTowardsPlayer(float deltaTime, const Player& player);
    bool isTileWalkable(int x, int y, const Level& level) const;
    
protected:
    float speed;
    Level& level;
    std::vector<sf::Vector2f> path;
    size_t pathIndex;
    float pathfindingRange;
    float recalculatePathThreshold = 32.0f;

    void updateCollisionRect();
};

    

#endif