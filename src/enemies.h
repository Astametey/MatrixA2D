#ifndef _ENEMIES_H_
#define _ENEMIES_H_

#include "entity.h"
#include "player.h"

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

    enum class AnimationType {
        Idle,       // Покой
        Walk,       // Ходьба 
        Attack,     // Атака 
        PowerAttack,// Усиленная Атака
        Hit,        // Получение урона
        Death       // Смерть
    };

    

    Enemy(float x, float y, float width, float height, Level& lvl,
        const std::string& texturePath = "resources/Entities/cat.png")
        : Entity(x, y, width, height), level(lvl) {
        shape.setFillColor(sf::Color::Magenta);
        speed = 25.0f;
        pathIndex = 0;
        pathfindingRange = 200.0f;
        bounceTime = 0.5f;
        currentState = AnimationType::Idle;
        attackCooldown = 0.0f;
        attackAreaSize = sf::Vector2f(16.0f, 16.0f); // Размер области атаки

        // Пытаемся загрузить текстуру из переданного пути
        if (!texture.loadFromFile(texturePath)) {
            // Если не удалось - используем запасной вариант (зеленый цвет)
            shape.setFillColor(sf::Color::Green);
        }
        else {
            sprite.setTexture(texture);
            frameWidth = width;  // Ширина одного кадра
            frameHeight = height; // Высота одного кадра
            framesPerAnimation = 6; // Например, 4 кадра на анимацию
            currentFrame = 0;
            animationSpeed = 0.1f; // 100ms
            currentAnimation = AnimationType::Idle;
            sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
            shape.setFillColor(sf::Color::Transparent);
        }

        sprite.setOrigin(shape.getGlobalBounds().width / 2, shape.getGlobalBounds().height / 2);

        health = maxHealth;
    }
    // Разрешаем только перемещение (опционально)
    Enemy(Enemy&&) = default;
    Enemy& operator=(Enemy&&) = default;


    ~Enemy() override {}

    // Метод для нанесения урона
    void takeDamage(int damage) {
        // Добавляем проверку на состояние Death
        if (isDead() || currentState == AnimationType::Death)
            return;

        health -= damage;
        std::cout << "Enemy took " << damage << " damage. Health: " << health << "/" << maxHealth << std::endl;

        if (health <= 0) {
            health = 0;
            std::cout << "Enemy died! Playing death animation..." << std::endl;
            setState(AnimationType::Death);
            // Отключаем коллизию сразу при смерти
            setCollisionSize(0, 0);
        }
        else {
            setState(AnimationType::Hit);
        }
    }

    void applyPushForce(const sf::Vector2f& force) {
        velocity = force * knockbackSlowdown;
    }

    void update(float deltaTime, const Player& player, const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
        if (isDead()) {
            updateAnimation();
            return;
        }

        // Обновляем таймер перезарядки атаки
        if (attackCooldown > 0) {
            attackCooldown -= deltaTime;
        }

        // Поворачиваемся к игроку
        sf::Vector2f toPlayer = player.getPosition() - position;
        isFacingRight = toPlayer.x > 0;

        // Проверяем расстояние до игрока для атаки
        float distanceToPlayer = calculateDistance(position, player.getPosition());
        bool canAttack = distanceToPlayer <= attackDistance && attackCooldown <= 0;

        // Обновление состояний
        if (stateTimer > 0) {
            stateTimer -= deltaTime;
        }
        else if (currentState == AnimationType::Attack || currentState == AnimationType::Hit) {
            // После атаки или получения урона проверяем дистанцию
            setState(canAttack ? AnimationType::Attack : AnimationType::Walk);
        }

        // Если можем атаковать и не в состоянии атаки - начинаем атаку
        if (canAttack && currentState != AnimationType::Attack) {
            setState(AnimationType::Attack);
            attackCooldown = 5.0f; // Устанавливаем перезарядку атаки
        }

        if (!isFacingRight) {
            sprite.setScale(-1.f, 1.f);
        }
        else {
            sprite.setScale(1.f, 1.f);
        }

        sprite.setPosition(position);

        // Обработка разных состояний
        switch (currentState) {
        case AnimationType::Idle:
            handleIdleState(deltaTime, player);
            break;
        case AnimationType::Walk:
        case AnimationType::Hit:
            handleChaseState(deltaTime, player);
            break;
        case AnimationType::Attack:
            handleAttackState(deltaTime, player);
            break;
        }

        pushOtherEnemies(allEnemies);

        if (player.isAttacking) {
            if (checkPlayerCollision(player)) {
                onPlayerCollision(player);
            }
        }

        Entity::update(deltaTime);

        // Обновление анимации
        updateAnimation();
    }

    // Getters
    sf::FloatRect getGlobalBounds() const override {
        if (texture.getSize().x > 0) {
            // Если есть текстура, используем границы спрайта
            sf::FloatRect bounds = sprite.getGlobalBounds();
            bounds.left = position.x - bounds.width / 2;
            bounds.top = position.y - bounds.height / 2;
            return bounds;
        }
        else {
            // Иначе используем стандартные границы
            return Entity::getGlobalBounds();
        }
    }

    // Setters
    void setHealth(int newHealth) {
        health = newHealth;
        if (health > maxHealth) health = maxHealth;
    }
    void setMaxHealth(int newMaxHealth) {
        maxHealth = newMaxHealth;
        health = maxHealth;
    }

    void setState(AnimationType newState) {
        if (currentState != newState) {
            currentFrame = 0;
            currentState = newState;
            currentAnimation = newState; // Синхронизируем состояние и анимацию

            // Сбрасываем флаг завершения анимации смерти, если вышли из состояния смерти
            if (newState != AnimationType::Death) {
                deathAnimationFinished = false;
            }

            switch (currentState) {
            case AnimationType::Hit:
                stateTimer = 0.3f;
                break;
            case AnimationType::Attack:
                stateTimer = 0.5f;
                velocity = sf::Vector2f(0, 0);
                break;
            case AnimationType::Death:
                // Особые настройки для смерти
                setCollisionSize(0, 0);
                velocity = sf::Vector2f(0, 0);
                break;
            case AnimationType::Idle:
                velocity = sf::Vector2f(0, 0);
                break;
            case AnimationType::Walk:
                velocity = sf::Vector2f(0, 0);
                break;
            }
        }
    }

    void setAnimation(AnimationType type) {
        if (currentAnimation != type) {
            currentAnimation = type;
            currentFrame = 0;
            animationClock.restart();
        }
    }

    void setCollisionSize(float width, float height) {
        collisionSize.x = width;
        collisionSize.y = height;
        updateCollisionRect(); // Обновляем прямоугольник коллизий
    }

    void setCollisionSize(const sf::Vector2f& newSize) {
        collisionSize = newSize;
        updateCollisionRect();
    }

    // Обновление анимации

    bool isDeathAnimationFinished() const {
        return deathAnimationFinished;
    }

    void updateAnimation() {
        if (animationClock.getElapsedTime().asSeconds() >= animationSpeed) {
            if (currentState == AnimationType::Death) {
                if (!deathAnimationFinished) {
                    if (currentFrame < framesPerAnimation - 1) {
                        currentFrame++;
                    }
                    else {
                        deathAnimationFinished = true; // Анимация завершена
                    }
                }
            }
            else {
                currentFrame = (currentFrame + 1) % framesPerAnimation;
            }

            int animationRow = static_cast<int>(currentState);
            sprite.setTextureRect(sf::IntRect(
                currentFrame * frameWidth,
                animationRow * frameHeight,
                frameWidth,
                frameHeight
            ));
            animationClock.restart();
        }
    }
    

    // Методы для обработки состояний
    void handleIdleState(float deltaTime, const Player& player) {
        velocity = sf::Vector2f(0, 0);

        // Проверяем расстояние до игрока
        float distance = calculateDistance(position, player.getPosition());
        if (distance <= pathfindingRange) {
            // Если игрок близко, но не достаточно для атаки - идем к нему
            if (distance > attackDistance) {
                setState(AnimationType::Walk);
            }
            // Если в радиусе атаки - сразу атакуем
            else {
                setState(AnimationType::Attack);
            }
        }
    }

    void handleChaseState(float deltaTime, const Player& player) {
        float distanceToPlayer = calculateDistance(position, player.getPosition());

        // Если игрок слишком далеко - idle
        if (distanceToPlayer > pathfindingRange * 1.5f) {
            setState(AnimationType::Idle);
            path.clear();
            return;
        }

        // Если в радиусе атаки - атакуем
        if (distanceToPlayer <= attackDistance) {
            setState(AnimationType::Attack);
            return;
        }

        // Проверяем, есть ли препятствия между врагом и игроком
        bool obstructed = isPathObstructed(player);

        // Если путь свободен - идём напрямую
        if (!obstructed) {
            sf::Vector2f direction = player.getPosition() - position;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (length > 0) {
                direction /= length;
                // Проверяем, можно ли двигаться в этом направлении
                sf::Vector2f nextPos = position + direction * speed * deltaTime;
                if (isPositionWalkable(nextPos)) {
                    velocity = direction * speed;
                    path.clear(); // Очищаем путь, так как идём напрямую
                    return;
                }
            }
        }

        // Если путь заблокирован или прямое движение невозможно - используем поиск пути
        if (path.empty() || calculateDistance(lastPlayerPosition, player.getPosition()) > recalculatePathThreshold) {
            calculatePath(player);
            lastPlayerPosition = player.getPosition();
        }

        // Двигаемся по пути, если он есть
        if (!path.empty()) {
            sf::Vector2f target = path[pathIndex];
            sf::Vector2f direction = target - position;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance < 5.0f) { // Достигли текущей точки пути
                if (pathIndex < path.size() - 1) {
                    pathIndex++;
                }
                else {
                    path.clear(); // Достигли конца пути
                }
            }
            else if (distance > 0) {
                direction /= distance;
                velocity = direction * speed;
            }
        }
    }

    void handleBounceState(float deltaTime) {
        // В режиме отскока просто продолжаем движение с текущей скоростью
        // (скорость была установлена при столкновении)
    }

    void handleAttackState(float deltaTime, const Player& player) {
        // Во время атаки враг не двигается
        velocity = sf::Vector2f(0, 0);

        // Проверяем, закончилась ли анимация атаки
        if (stateTimer <= 0) {
            float distance = calculateDistance(position, player.getPosition());
            setState(distance <= attackDistance ? AnimationType::Attack : AnimationType::Walk);
        }
    }

    

    // Проверка столкновения с игроком
    bool checkPlayerCollision(const Player& player) const {
        // Всегда проверяем столкновение с областью атаки, если игрок атакует
        if (player.isAttacking) {
            sf::FloatRect attackBounds = player.getAttackAreaBounds();
            sf::FloatRect enemyBounds = this->getGlobalBounds();
            if (attackBounds.intersects(enemyBounds)) {
                return true;
            }
        }

        // Проверяем обычное столкновение с игроком
        sf::FloatRect enemyBounds = this->getGlobalBounds();
        sf::FloatRect playerBounds = player.getGlobalBounds();
        return enemyBounds.intersects(playerBounds);
    }

    // Обработка столкновения с игроком
    void onPlayerCollision(const Player& player) {
        if (player.isAttacking) {
            sf::FloatRect attackBounds = player.getAttackAreaBounds();
            sf::FloatRect enemyBounds = this->getGlobalBounds();

            if (attackBounds.intersects(enemyBounds)) {
                if (!wasHitThisAttack) {
                    takeDamage(player.getAttackDamage());
                    wasHitThisAttack = true;

                    // Рассчитываем направление отталкивания от центра атаки
                    sf::Vector2f attackCenter(
                        attackBounds.left + attackBounds.width / 2,
                        attackBounds.top + attackBounds.height / 2
                    );

                    sf::Vector2f direction = position - attackCenter;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0) {
                        direction /= length;
                    }

                    // Увеличиваем силу отталкивания
                    float pushForce = 3000.0f; // Можно регулировать это значение
                    velocity = direction * pushForce;

                    // Устанавливаем состояние Hit, только если враг еще жив
                    if (!isDead()) {
                        setState(AnimationType::Hit);
                    }

                    return;
                }
            }
        }
        else {
            wasHitThisAttack = false;
            if (!isDead()) {
                setState(AnimationType::Attack);
            }
        }
    }

    // Метод для рисования пути (для отладки)
    void drawPath(sf::RenderTarget& target, sf::RenderStates states) const {
        if (!path.empty()) {
            sf::VertexArray lines(sf::LineStrip, path.size());
            for (size_t i = 0; i < path.size(); ++i) {
                lines[i].position = path[i];
                lines[i].color = sf::Color::Yellow;
            }
            target.draw(lines, states);
        }
    }

    bool isPositionWalkable(const sf::Vector2f& pos) const {
        // Проверяем все 4 угла коллизионного прямоугольника
        sf::Vector2f halfSize = collisionSize / 2.0f;

        std::vector<sf::Vector2f> points = {
            sf::Vector2f(pos.x - halfSize.x, pos.y - halfSize.y), // верхний левый
            sf::Vector2f(pos.x + halfSize.x, pos.y - halfSize.y), // верхний правый
            sf::Vector2f(pos.x - halfSize.x, pos.y + halfSize.y), // нижний левый
            sf::Vector2f(pos.x + halfSize.x, pos.y + halfSize.y)  // нижний правый
        };

        for (const auto& point : points) {
            int tileX = static_cast<int>(point.x / level.tileWidth);
            int tileY = static_cast<int>(point.y / level.tileHeight);

            if (!isTileWalkable(tileX, tileY, level)) {
                return false;
            }
        }

        return true;
    }

    void pushOtherEnemies(const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
        for (const auto& other : allEnemies) {
            if (other.get() == this) continue; // Не толкаем себя

            float distance = calculateDistance(position, other->position);
            if (distance < MIN_PUSH_DISTANCE && distance > 0) {
                sf::Vector2f pushDir = position - other->position;
                pushDir /= distance;

                float force = PUSH_FORCE * (1.0f - distance / MIN_PUSH_DISTANCE);

                velocity += pushDir * force;
                // Для изменения скорости другого врага нужно сделать метод setVelocity
                other->applyPushForce(-pushDir * force);
            }
        }
    }

private:
    int health = 3; // Начальное здоровье
    bool deathAnimationPlayed = false; // Флаг завершения анимации смерти
    bool deathAnimationFinished = false; // Добавьте это

    float attackDistance = 14.0f; // Уменьшаем дистанцию атаки
    float stopDistance = 18.0f;   // Дистанция остановки чуть больше атаки

    bool wasHitThisAttack = false; // Добавьте это в приватные поля
    float knockbackSlowdown = 0.5f; // Коэффициент замедления (0.5 = 50% скорости)
    

    // Добавляем новые приватные переменные
    float attackCooldown;       // Таймер перезарядки атаки
    sf::Vector2f attackAreaSize; // Размер области атаки

    static constexpr float PUSH_FORCE = 50.0f; // Сила толкания между врагами
    static constexpr float MIN_PUSH_DISTANCE = 16.0f; // Минимальное расстояние для толкания
    static constexpr float WALL_CHECK_DISTANCE = 8.0f; // Дистанция проверки стен при отскоке

    AnimationType currentState;
    float stateTimer;       // Таймер для временных состояний
    float bounceTime;       // Длительность отскока
    // Анимационные переменные
    sf::Texture texture;
    sf::Sprite sprite;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    
    AnimationType currentAnimation;
    sf::Clock animationClock;
    float animationSpeed; // Время между кадрами в секундах
    bool isFacingRight;



    sf::Vector2f lastPlayerPosition;
    float pathUpdateDistance = 64.0f; // Расстояние, которое должен пройти игрок для обновления пути
    // Helper function to calculate distance
    float calculateDistance(const sf::Vector2f& pos1, const sf::Vector2f& pos2) const {
        float dx = pos1.x - pos2.x;
        float dy = pos1.y - pos2.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Функция для проверки, есть ли препятствия между врагом и игроком
    bool isPathObstructed(const Player& player) const {
        sf::FloatRect enemyRect = shape.getGlobalBounds();

        std::vector<sf::Vector2f> checkPoints = {
            sf::Vector2f(enemyRect.left, enemyRect.top),
            sf::Vector2f(enemyRect.left + enemyRect.width, enemyRect.top),
            sf::Vector2f(enemyRect.left, enemyRect.top + enemyRect.height),
            sf::Vector2f(enemyRect.left + enemyRect.width, enemyRect.top + enemyRect.height)
        };

        int obstructedCount = 1; // Количество углов, для которых путь прегражден

        for (const auto& startPoint : checkPoints) {
            sf::Vector2f direction = player.getPosition() - startPoint;
            float distance = calculateDistance(startPoint, player.getPosition());
            int numSteps = static_cast<int>(distance / (level.tileWidth / 4.0f));
            if (numSteps <= 0) numSteps = 1;

            bool obstructed = false;
            for (int i = 1; i <= numSteps; ++i) {
                float t = static_cast<float>(i) / numSteps;
                sf::Vector2f checkPoint = startPoint + direction * t;

                int tileX = static_cast<int>(checkPoint.x / level.tileWidth);
                int tileY = static_cast<int>(checkPoint.y / level.tileHeight);

                if (!isTileWalkable(tileX, tileY, level)) {
                    obstructed = true;
                    break;
                }
            }

            if (obstructed) {
                obstructedCount++;
            }
        }

        // Разрешаем одному углу быть прегражденным
        return obstructedCount > 1; // Если больше одного угла преграждены, считаем путь прегражденным
    }

    // Функция для расчета пути
    void calculatePath(const Player& player) {
        sf::Vector2f playerPosition = player.getPosition();

        // Добавляем небольшой отступ от позиции игрока (например, 5 пикселей)
        sf::Vector2f direction = position - playerPosition;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction /= length;
        }
        playerPosition -= direction * 5.0f; // Отодвигаем на 5 пикселей

        path = findPath(position, playerPosition, level);
        pathIndex = 0;
    }

    // Функция для движения прямо к игроку
    void moveTowardsPlayer(float deltaTime, const Player& player) {
        sf::Vector2f direction = player.getPosition() - position;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length > 0) {
            direction /= length;
        }

        velocity = direction * speed;
        Entity::update(deltaTime);
    }

    bool isTileWalkable(int x, int y, const Level& level) const {
        // Проверяем, находится ли тайл в пределах карты
        if (x < 0 || x >= level.width || y < 0 || y >= level.height) {
            return false; // За пределами карты = непроходимо
        }

        // Получаем GID тайла на данном слое (предполагаем, что используется первый слой)
        if (level.layers.empty()) {
            return false; // Нет слоев = нечего проверять
        }

        int tileGID = level.layers[0].tileGID[x + y * level.width];

        // Проверяем, является ли тайл "твердым" на основе информации об объектах
        for (const auto& object : level.getObjects()) {
            // Проверяем, находится ли тайл в прямоугольнике объекта
            if (object.type == "solid" &&
                x * level.tileWidth >= object.rect.left && x * level.tileWidth < object.rect.left + object.rect.width &&
                y * level.tileHeight >= object.rect.top && y * level.tileHeight < object.rect.top + object.rect.height) {
                return false; // Тайл находится внутри "твердого" объекта = непроходим
            }
        }

        // Если тайл не найден как "твердый", считаем его проходимым
        return true;
    }

protected:
    float speed;
    Level& level;
    std::vector<sf::Vector2f> path;
    size_t pathIndex;
    float pathfindingRange; // Дальность поиска пути
    float recalculatePathThreshold = 32.0f;

    void updateCollisionRect() {
        rect.left = position.x - collisionSize.x / 2;
        rect.top = position.y - collisionSize.y / 2;
        rect.width = collisionSize.x;
        rect.height = collisionSize.y;
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Рисуем спрайт в любом случае (даже для мертвых врагов)
        if (texture.getSize().x > 0) {
            target.draw(sprite, states);
        }
        else {
            sf::RectangleShape debugRect(visualSize);
            debugRect.setPosition(position);
            debugRect.setOrigin(visualSize.x / 2, visualSize.y / 2);
            debugRect.setFillColor(sf::Color::Green);
            target.draw(debugRect, states);
        }

        // Рисуем остальные элементы только для живых врагов
        if (!isDead()) {
            // Полоска здоровья
            float clampedHealth = std::max(0.0f, std::min(static_cast<float>(health), static_cast<float>(maxHealth)));
            const float healthBarWidth = 30.0f;
            const float healthBarHeight = 5.0f;

            sf::RectangleShape healthBar(sf::Vector2f(healthBarWidth, healthBarHeight));
            healthBar.setPosition(position.x - healthBarWidth / 2, position.y - 30);
            healthBar.setFillColor(sf::Color::Red);

            float fillWidth = healthBarWidth * (clampedHealth / maxHealth);
            sf::RectangleShape healthBarFill(sf::Vector2f(fillWidth, healthBarHeight));
            healthBarFill.setPosition(position.x - healthBarWidth / 2, position.y - 30);
            healthBarFill.setFillColor(sf::Color::Green);
            healthBar.setOutlineThickness(1);
            healthBar.setOutlineColor(sf::Color::Black);

            target.draw(healthBar, states);
            target.draw(healthBarFill, states);

            // Хитбокс
            sf::RectangleShape hitbox(collisionSize);
            hitbox.setPosition(position);
            hitbox.setOrigin(collisionSize.x / 2, collisionSize.y / 2);
            hitbox.setFillColor(sf::Color::Transparent);
            hitbox.setOutlineThickness(1);
            target.draw(hitbox, states);

            // Область атаки
            if (currentState == AnimationType::Attack) {
                sf::RectangleShape attackArea(attackAreaSize);
                attackArea.setPosition(position);
                attackArea.setOrigin(isFacingRight ?
                    sf::Vector2f(-collisionSize.x / 2, attackAreaSize.y / 2) :
                    sf::Vector2f(collisionSize.x / 2 + attackAreaSize.x, attackAreaSize.y / 2));
                attackArea.setFillColor(sf::Color(255, 0, 0, 100));
                target.draw(attackArea, states);
            }
        }

        // Рисуем путь
        if (!path.empty()) {
            sf::VertexArray lines(sf::LinesStrip, path.size());
            for (size_t i = 0; i < path.size(); ++i) {
                lines[i].position = path[i];
                lines[i].color = (i == pathIndex) ? sf::Color::Red : sf::Color::Green;
            }
            target.draw(lines, states);

            // Рисуем текущую цель
            sf::CircleShape targetMarker(3);
            targetMarker.setFillColor(sf::Color::Red);
            targetMarker.setOrigin(3, 1.5);
            targetMarker.setPosition(path[pathIndex]);
            target.draw(targetMarker, states);
        }

    }
};


#endif 