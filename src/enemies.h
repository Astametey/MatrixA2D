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
        Death,      // Смерть
        Bounce      // Отталкивание
    };



    Enemy(float x, float y, float width, float height, Level& lvl,
        const std::string& texturePath = "resources/Entities/cat.png")
        : Entity(x, y, width, height), level(lvl) {
        pushForce = 300.0f;
        bounceDuration = 0.2f;
        currentBounceTime = 0.0f;
        bounceVelocity = sf::Vector2f(0, 0);

        shape.setFillColor(sf::Color::Magenta);
        speed = 25.0f;
        pathIndex = 0;
        pathfindingRange = 200.0f;
        bounceTime = 0.5f;
        currentState = AnimationType::Idle;
        attackCooldown = 0.0f;
        attackAreaSize = sf::Vector2f(16.0f, 16.0f); // Размер области атаки

        deathAnimationFinished = false;
        currentAnimation = AnimationType::Idle;

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
    void Set_type_enemy(std::string type)
    {
        if (type == "orc") {
         setCollisionSize(16, 16);
         setMaxHealth(10);
         
         if (!texture.loadFromFile("resources/Entities/Orc-Sheet.png")) {
             // Если не удалось - используем запасной вариант (зеленый цвет)
             shape.setFillColor(sf::Color::Green);
         }
         else {
             sprite.setTexture(texture);
             frameWidth = 38;  // Ширина одного кадра
             frameHeight = 33; // Высота одного кадра
             framesPerAnimation = 6; // Например, 4 кадра на анимацию
             currentFrame = 0;
             animationSpeed = 0.1f; // 100ms
             currentAnimation = AnimationType::Idle;
             sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
             shape.setFillColor(sf::Color::Transparent);
         }


        };
    };

    // Метод для нанесения урона
    void takeDamage(int damage) {
        if (isDead() || currentState == AnimationType::Death)
            return;

        health -= damage;

        if (health <= 0) {
            health = 0;
            setState(AnimationType::Death);
            setAnimation(AnimationType::Death); // Явно устанавливаем анимацию
            setCollisionSize(0, 0);
            currentFrame = 0;
            deathAnimationFinished = false;
            velocity = sf::Vector2f(0, 0); // Останавливаем движение
        }
        else {
            setState(AnimationType::Hit);
            setAnimation(AnimationType::Hit);
        }
    }

    void applyPushForce(const sf::Vector2f& force) {
        velocity = force * knockbackSlowdown;
    }

    void update(float deltaTime, const Player& player, const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
        if (isDead()) {
            if (currentState != AnimationType::Death) {
                setState(AnimationType::Death);
            }
            updateAnimation();
            return;
        }

        // Обработка отталкивания
        if (currentState == AnimationType::Bounce) {
            currentBounceTime -= deltaTime;
            if (currentBounceTime <= 0) {
                setState(AnimationType::Walk);
            }
            else {
                // Применяем замедление отталкивания
                float slowdown = currentBounceTime / bounceDuration;
                sf::Vector2f newVelocity = bounceVelocity * slowdown;

                // Проверяем следующую позицию
                sf::Vector2f newPos = position + newVelocity * deltaTime;

                if (!checkCollisionWithSolids(newPos)) {
                    // Если нет столкновения - двигаемся
                    velocity = newVelocity;
                    Entity::update(deltaTime);
                }
                else {
                    // Если есть столкновение - останавливаем отталкивание
                    currentBounceTime = 0;
                    setState(AnimationType::Walk);
                }

                sprite.setPosition(position);
                updateAnimation();
                return;
            }
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

        sprite.setPosition(position);

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
            currentAnimation = newState; // Синхронизируем анимацию с состоянием

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
            case AnimationType::Bounce:
                currentAnimation = AnimationType::Hit; // Или создайте отдельную анимацию для отталкивания
                stateTimer = bounceDuration;
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
            if (currentAnimation == AnimationType::Death) {
                if (!deathAnimationFinished) {
                    if (currentFrame < framesPerAnimation - 1) {
                        currentFrame++;
                    }
                    else {
                        deathAnimationFinished = true;
                    }
                }
            }
            else {
                currentFrame = (currentFrame + 1) % framesPerAnimation;
            }

            // Всегда обновляем текстуру, даже для смерти
            int animationRow = static_cast<int>(currentAnimation);
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
                sf::Vector2f nextPos = position + direction * speed * deltaTime;
                if (isPositionWalkable(nextPos)) {
                    velocity = direction * speed;
                    path.clear();
                    return;
                }
            }
        }

        // Проверяем, нужно ли пересчитать путь
        bool shouldRecalculate = path.empty() ||
            calculateDistance(lastPlayerPosition, player.getPosition()) > recalculatePathThreshold;

        // Если нужно пересчитать путь
        if (shouldRecalculate) {
            // Рассчитываем новый путь до игрока
            auto newPath = findPath(position, player.getPosition(), level);

            if (!newPath.empty()) {
                // Если у нас был предыдущий путь, пытаемся сохранить прогресс
                if (!path.empty() && pathIndex > 0) {
                    // Ищем ближайшую точку в новом пути к нашей текущей цели
                    size_t closestIndex = 0;
                    float minDistance = std::numeric_limits<float>::max();

                    // Начинаем поиск со второй точки (индекс 1), чтобы не возвращаться в начало
                    for (size_t i = 1; i < newPath.size(); ++i) {
                        float dist = calculateDistance(path[pathIndex], newPath[i]);
                        if (dist < minDistance) {
                            minDistance = dist;
                            closestIndex = i;
                        }
                    }

                    // Если нашли подходящую точку, продолжаем с нее
                    if (minDistance < level.tileWidth * 1.5f) { // Пороговое расстояние = 1.5 тайла
                        pathIndex = closestIndex;
                    }
                    else {
                        pathIndex = 1; // Начинаем со второй точки, если не нашли подходящей
                    }
                }
                else {
                    pathIndex = 1; // Начинаем сразу со второй точки пути
                }

                path = newPath;
                lastPlayerPosition = player.getPosition();
            }
        }

        // Двигаемся по пути
        if (!path.empty()) {
            // Если мы на последней точке пути, но игрок ушел дальше - очищаем путь
            if (pathIndex >= path.size() - 1 && distanceToPlayer > recalculatePathThreshold) {
                path.clear();
                return;
            }

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
        // Проверяем столкновение с оружием, если игрок атакует и имеет оружие
        if (player.isAttacking && player.isMeleeEquipped()) {
            sf::FloatRect weaponBounds = player.meleeWeapon.sprite.getGlobalBounds();
            sf::FloatRect enemyBounds = this->getGlobalBounds();
            if (weaponBounds.intersects(enemyBounds)) {
                return true;
            }
        }
        // Проверяем столкновение с рукой, если игрок атакует, но оружие не экипировано
        else if (player.isAttacking && !player.isMeleeEquipped()) {
            sf::FloatRect handBounds = player.frontHandSprite.getGlobalBounds(); // Берем переднюю руку
            sf::FloatRect enemyBounds = this->getGlobalBounds();
            if (handBounds.intersects(enemyBounds)) {
                return true;
            }
        }

        // Проверяем обычное столкновение с игроком (как и раньше)
        sf::FloatRect enemyBounds = this->getGlobalBounds();
        sf::FloatRect playerBounds = player.getGlobalBounds();
        return enemyBounds.intersects(playerBounds);
    }
    bool checkCollisionWithSolids(const sf::Vector2f& newPos) const {
        sf::FloatRect newBounds(newPos.x - collisionSize.x / 2, newPos.y - collisionSize.y / 2,
            collisionSize.x, collisionSize.y);

        // Проверяем все углы нового положения
        std::vector<sf::Vector2f> points = {
            {newBounds.left, newBounds.top},
            {newBounds.left + newBounds.width, newBounds.top},
            {newBounds.left, newBounds.top + newBounds.height},
            {newBounds.left + newBounds.width, newBounds.top + newBounds.height}
        };

        for (const auto& point : points) {
            int tileX = static_cast<int>(point.x / level.tileWidth);
            int tileY = static_cast<int>(point.y / level.tileHeight);

            if (!isTileWalkable(tileX, tileY, level)) {
                return true;
            }
        }

        return false;
    }

    // Обработка столкновения с игроком
    void onPlayerCollision(const Player& player) {
        // Обработка столкновения с оружием, если оно есть
        if (player.isAttacking && player.isMeleeEquipped()) {
            sf::FloatRect weaponBounds = player.meleeWeapon.sprite.getGlobalBounds();
            sf::FloatRect enemyBounds = this->getGlobalBounds();

            if (weaponBounds.intersects(enemyBounds)) {
                if (!wasHitThisAttack) {
                    takeDamage(player.getAttackDamage());
                    wasHitThisAttack = true;

                    // Рассчитываем направление отталкивания от оружия
                    sf::Vector2f weaponCenter(
                        weaponBounds.left + weaponBounds.width / 2,
                        weaponBounds.top + weaponBounds.height / 2
                    );

                    sf::Vector2f direction = position - weaponCenter;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0) {
                        direction /= length;
                    }

                    // Устанавливаем параметры отталкивания
                    bounceVelocity = direction * pushForce;
                    currentBounceTime = bounceDuration;
                    setState(AnimationType::Bounce);
                    setAnimation(AnimationType::Hit);
                    return;
                }
            }
        }
        // Обработка столкновения с рукой, если оружие отсутствует
        else if (player.isAttacking && !player.isMeleeEquipped()) {
            sf::FloatRect handBounds = player.frontHandSprite.getGlobalBounds();
            sf::FloatRect enemyBounds = this->getGlobalBounds();

            if (handBounds.intersects(enemyBounds)) {
                if (!wasHitThisAttack) {
                    takeDamage(player.getBaseDamage(false)); // Уменьшенный урон рукой
                    wasHitThisAttack = true;

                    // Рассчитываем направление отталкивания от руки
                    sf::Vector2f handCenter(
                        handBounds.left + handBounds.width / 2,
                        handBounds.top + handBounds.height / 2
                    );

                    sf::Vector2f direction = position - handCenter;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0) {
                        direction /= length;
                    }

                    // Устанавливаем параметры отталкивания
                    bounceVelocity = direction * (pushForce / 2); // Уменьшенное отталкивание
                    currentBounceTime = bounceDuration;
                    setState(AnimationType::Bounce);
                    setAnimation(AnimationType::Hit);
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

    bool wasHitThisAttack = false;
    float knockbackSlowdown = 0.5f; // Коэффициент замедления (0.5 = 50% скорости)

    float pushForce = 0.0f; // Увеличьте это значение по необходимости
    float bounceDuration = 0.3f; // Длительность отталкивания
    float currentBounceTime = 0.0f;
    sf::Vector2f bounceVelocity;

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

        if (!(isDead() && deathAnimationFinished)) {
            if (texture.getSize().x > 0) {
                target.draw(sprite, states);
            }
            else {
                //sf::RectangleShape debugRect(visualSize);
                //debugRect.setPosition(position);
                //debugRect.setOrigin(visualSize.x / 2, visualSize.y / 2);
                //debugRect.setFillColor(sf::Color::Green);
                //target.draw(debugRect, states);
            }
        }

        if (!isDead()) {
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
            // *************** Изменено ***************
            if (currentState == AnimationType::Attack &&
                currentFrame >= framesPerAnimation / 2) // Проверяем, что кадр находится в середине анимации
            {
                sf::RectangleShape attackArea(attackAreaSize);
                attackArea.setPosition(position);
                attackArea.setOrigin(isFacingRight ?
                    sf::Vector2f(-collisionSize.x / 2, attackAreaSize.y / 2) :
                    sf::Vector2f(collisionSize.x / 2 + attackAreaSize.x, attackAreaSize.y / 2));
                attackArea.setFillColor(sf::Color(255, 0, 0, 100));
                target.draw(attackArea, states);
            }
            // *************** Конец изменений ***************
        };

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
