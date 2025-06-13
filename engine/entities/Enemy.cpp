#include "Enemy.h"

Enemy::Enemy(float x, float y, float width, float height, Level& lvl, const std::string& texturePath)
    : Entity(x, y, width, height), level(lvl) {
    bounceDuration = 0.5f;
    currentBounceTime = 0.0f;
    bounceVelocity = sf::Vector2f(0, 0);

    shape.setFillColor(sf::Color::Magenta);
    speed = 25.0f;
    pathIndex = 0;
    pathfindingRange = 200.0f;
    bounceTime = 0.5f;
    currentState = AnimationType::Idle;
    attackCooldown = 0.0f;
    attackAreaSize = sf::Vector2f(16.0f, 16.0f);

    deathAnimationFinished = false;
    currentAnimation = AnimationType::Idle;

    if (!texture.loadFromFile(texturePath)) {
        shape.setFillColor(sf::Color::Green);
    }
    else {
        sprite.setTexture(texture);
        frameWidth = width;
        frameHeight = height;
        framesPerAnimation = 6;
        currentFrame = 0;
        animationSpeed = 0.1f;
        currentAnimation = AnimationType::Idle;
        sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
        shape.setFillColor(sf::Color::Transparent);
    }

    sprite.setOrigin(shape.getGlobalBounds().width / 2, shape.getGlobalBounds().height / 2);
    health = maxHealth;
    Set_type_enemy("cat");
}

void Enemy::Set_type_enemy(std::string type) {
    if (type == "orc") {
        setCollisionSize(16, 16);
        setMaxHealth(10);

        if (!texture.loadFromFile("resources/textures/entities/enemies/Orc-Sheet.png")) {
            shape.setFillColor(sf::Color::Green);
        }
        else {
            sprite.setTexture(texture);
            frameWidth = 38;
            frameHeight = 33;
            framesPerAnimation = 6;
            currentFrame = 0;
            animationSpeed = 0.1f;
            currentAnimation = AnimationType::Idle;
            sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
            shape.setFillColor(sf::Color::Transparent);
            collisionSize.x = 16;
            collisionSize.y = 16;

        }
    }
    if (type == "soldier") {
        setCollisionSize(16, 16);
        setMaxHealth(15);

        if (!texture.loadFromFile("resources/textures/entities/enemies/Soldier-Sheet.png")) {
            shape.setFillColor(sf::Color::Green);
        }
        else {
            sprite.setTexture(texture);
            frameWidth = 34;
            frameHeight = 33;
            framesPerAnimation = 6;
            currentFrame = 0;
            animationSpeed = 0.1f;
            currentAnimation = AnimationType::Idle;
            sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
            shape.setFillColor(sf::Color::Transparent);
            collisionSize.x = 16;
            collisionSize.y = 16;

        }
    }
    if (type == "cat") {
        setCollisionSize(16, 16);
        setMaxHealth(5);

        if (!texture.loadFromFile("resources/textures/entities/enemies/cat.png")) {
            shape.setFillColor(sf::Color::Green);
        }
        else {
            sprite.setTexture(texture);
            frameWidth = 32;
            frameHeight = 32;
            framesPerAnimation = 5;
            currentFrame = 0;
            animationSpeed = 0.1f;
            currentAnimation = AnimationType::Idle;
            sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
            shape.setFillColor(sf::Color::Transparent);
            collisionSize.x = 16;
            collisionSize.y = 16;
        }
    }
    sprite.setOrigin(frameWidth / 2, frameHeight / 2);
}

void Enemy::takeDamage(int damage, int weaponDMG) {
    if (isDead() || currentState == AnimationType::Death)
        return;
    bool isCritical = false;
    if (damage >  weaponDMG)
    {
        isCritical = true;
    }
    

    health -= damage;
    addDamageText(damage, isCritical);


    if (health <= 0) {
        health = 0;
        setState(AnimationType::Death);  // Устанавливаем состояние смерти
        setAnimation(AnimationType::Death); // Запускаем анимацию смерти
        setCollisionSize(0, 0); // Отключаем коллизию
        currentFrame = 0; // Сбрасываем кадр анимации
        deathAnimationFinished = false; // Сбрасываем флаг завершения анимации
        velocity = sf::Vector2f(0, 0); // Останавливаем движение
    }
    else {
        setState(AnimationType::Hit);
        setAnimation(AnimationType::Hit);
    }
}

void Enemy::applyPushForce(const sf::Vector2f& force) {
    velocity = force * knockbackSlowdown;
}

void Enemy::update(float deltaTime, Player& player, const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
    if (isDead()) {
        if (currentState != AnimationType::Death) {
            setState(AnimationType::Death);
        }
        updateAnimation();
        return;
    }

    if (currentState == AnimationType::Bounce) {
        currentBounceTime -= deltaTime;
        if (currentBounceTime <= 0) {
            setState(AnimationType::Walk);
        }
        else {
            float slowdown = currentBounceTime / bounceDuration;
            sf::Vector2f newVelocity = bounceVelocity * slowdown;
            sf::Vector2f newPos = position + newVelocity * deltaTime;

            if (!checkCollisionWithSolids(newPos)) {
                velocity = newVelocity;
                Entity::update(deltaTime);
            }
            else {
                currentBounceTime = 0;
                setState(AnimationType::Walk);
            }

            sprite.setPosition(position);
            updateAnimation();
            return;
        }
    }

    if (attackCooldown > 0) {
        attackCooldown -= deltaTime;
    }

    sf::Vector2f toPlayer = player.getPosition() - position;
    isFacingRight = toPlayer.x > 0;

    float distanceToPlayer = calculateDistance(position, player.getPosition());
    bool canAttack = distanceToPlayer <= attackDistance && attackCooldown <= 0;

    if (stateTimer > 0) {
        stateTimer -= deltaTime;
    }
    else if (currentState == AnimationType::Attack || currentState == AnimationType::Hit) {
        setState(canAttack ? AnimationType::Attack : AnimationType::Walk);
    }

    // Проверка столкновения со снарядами игрока
    for (auto& projectile : player.getProjectiles()) {
        if (!projectile.hit && getGlobalBounds().intersects(projectile.shape.getGlobalBounds())) {
            takeDamage(player.getAttackDamage(), player.rangedWeapon.damage * 2);

            // Применяем отталкивание от снаряда
            sf::Vector2f direction = position - projectile.shape.getPosition();
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if (length > 0) {
                direction /= length;
            }

            bounceVelocity = direction * player.pushForce;
            currentBounceTime = bounceDuration;
            applyPushForce(direction * player.rangedWeapon.pushForce);
            currentBounceTime = bounceDuration;
            setState(AnimationType::Bounce);
            setAnimation(AnimationType::Hit);

            // Помечаем снаряд как попавший
            projectile.hit = true;
        }
    }


    if (canAttack && currentState != AnimationType::Attack) {
        setState(AnimationType::Attack);
        attackCooldown = 5.0f;
    }

    if (!isFacingRight) {
        sprite.setScale(-1.f, 1.f);
    }
    else {
        sprite.setScale(1.f, 1.f);
    }

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
        // Всегда проверяем столкновение с оружием при атаке
        onPlayerCollision(player);
    }
    else {
        // Сбрасываем флаг попадания, когда игрок не атакует
        wasHitThisAttack = false;
    }

    sprite.setPosition(position);
    Entity::update(deltaTime);
    updateAnimation();
    updateDamageTexts(deltaTime);
}

sf::FloatRect Enemy::getGlobalBounds() const {
    if (texture.getSize().x > 0) {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        bounds.left = position.x - bounds.width / 2;
        bounds.top = position.y - bounds.height / 2;
        return bounds;
    }
    else {
        return Entity::getGlobalBounds();
    }
}

void Enemy::setHealth(int newHealth) {
    health = newHealth;
    if (health > maxHealth) health = maxHealth;
}

void Enemy::setMaxHealth(int newMaxHealth) {
    maxHealth = newMaxHealth;
    health = maxHealth;
}

void Enemy::setState(AnimationType newState) {
    if (currentState != newState) {
        currentFrame = 0;
        currentState = newState;
        currentAnimation = newState;

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
            currentAnimation = AnimationType::Hit;
            stateTimer = bounceDuration;
            break;
        }
    }
}

void Enemy::setAnimation(AnimationType type) {
    if (currentAnimation != type) {
        currentAnimation = type;
        currentFrame = 0;
        animationClock.restart();
    }
}

void Enemy::setCollisionSize(float width, float height) {
    collisionSize.x = width;
    collisionSize.y = height;
    updateCollisionRect();
}

void Enemy::setCollisionSize(const sf::Vector2f& newSize) {
    collisionSize = newSize;
    updateCollisionRect();
}

bool Enemy::isDeathAnimationFinished() const {
    return deathAnimationFinished;
}

void Enemy::updateAnimation() {
    if (animationClock.getElapsedTime().asSeconds() >= animationSpeed) {
        if (currentAnimation == AnimationType::Death) {
            if (!deathAnimationFinished) {
                if (currentFrame < framesPerAnimation - 1) {
                    currentFrame++;
                }
                else {
                    deathAnimationFinished = true; // Анимация смерти завершена
                }
            }
        }
        else {
            currentFrame = (currentFrame + 1) % framesPerAnimation;
        }

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

void Enemy::handleIdleState(float deltaTime, const Player& player) {
    velocity = sf::Vector2f(0, 0);
    float distance = calculateDistance(position, player.getPosition());
    if (distance <= pathfindingRange) {
        if (distance > attackDistance) {
            setState(AnimationType::Walk);
        }
        else {
            setState(AnimationType::Attack);
        }
    }
}

void Enemy::handleChaseState(float deltaTime, const Player& player) {
    float distanceToPlayer = calculateDistance(position, player.getPosition());

    if (distanceToPlayer > pathfindingRange * 1.5f) {
        setState(AnimationType::Idle);
        path.clear();
        return;
    }

    if (distanceToPlayer <= attackDistance) {
        setState(AnimationType::Attack);
        return;
    }

    bool obstructed = isPathObstructed(player);

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

    bool shouldRecalculate = path.empty() ||
        calculateDistance(lastPlayerPosition, player.getPosition()) > recalculatePathThreshold;

    if (shouldRecalculate) {
        auto newPath = findPath(position, player.getPosition(), level);

        if (!newPath.empty()) {
            if (!path.empty() && pathIndex > 0) {
                size_t closestIndex = 0;
                float minDistance = std::numeric_limits<float>::max();

                for (size_t i = 1; i < newPath.size(); ++i) {
                    float dist = calculateDistance(path[pathIndex], newPath[i]);
                    if (dist < minDistance) {
                        minDistance = dist;
                        closestIndex = i;
                    }
                }

                if (minDistance < level.tileWidth * 1.5f) {
                    pathIndex = closestIndex;
                }
                else {
                    pathIndex = 1;
                }
            }
            else {
                pathIndex = 1;
            }

            path = newPath;
            lastPlayerPosition = player.getPosition();
        }
    }

    if (!path.empty()) {
        if (pathIndex >= path.size() - 1 && distanceToPlayer > recalculatePathThreshold) {
            path.clear();
            return;
        }

        sf::Vector2f target = path[pathIndex];
        sf::Vector2f direction = target - position;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance < 5.0f) {
            if (pathIndex < path.size() - 1) {
                pathIndex++;
            }
            else {
                path.clear();
            }
        }
        else if (distance > 0) {
            direction /= distance;
            velocity = direction * speed;
        }
    }
}

void Enemy::handleBounceState(float deltaTime) {
    // В режиме отскока просто продолжаем движение с текущей скоростью
}

void Enemy::handleAttackState(float deltaTime, const Player& player) {
    velocity = sf::Vector2f(0, 0);
    if (stateTimer <= 0) {
        float distance = calculateDistance(position, player.getPosition());
        setState(distance <= attackDistance ? AnimationType::Attack : AnimationType::Walk);
    }
}

bool Enemy::checkPlayerCollision(const Player& player) const {
    if (player.isAttacking && player.isMeleeEquipped()) {
        sf::FloatRect weaponBounds = player.meleeWeapon.sprite.getGlobalBounds();
        sf::FloatRect enemyBounds = this->getGlobalBounds();
        if (weaponBounds.intersects(enemyBounds)) {
            return true;
        }
    }
    else if (player.isAttacking && !player.isMeleeEquipped()) {
        sf::FloatRect handBounds = player.frontHandSprite.getGlobalBounds();
        sf::FloatRect enemyBounds = this->getGlobalBounds();
        if (handBounds.intersects(enemyBounds)) {
            return true;
        }
    }

    sf::FloatRect enemyBounds = this->getGlobalBounds();
    sf::FloatRect playerBounds = player.getGlobalBounds();
    return enemyBounds.intersects(playerBounds);
}

bool Enemy::checkCollisionWithSolids(const sf::Vector2f& newPos) const {
    sf::FloatRect newBounds(newPos.x - collisionSize.x / 2, newPos.y - collisionSize.y / 2,
        collisionSize.x, collisionSize.y);

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
bool Enemy::shouldDraw(const sf::FloatRect& viewBounds) const {
    return (!isDead() || !isDeathAnimationFinished()) &&
        getGlobalBounds().intersects(viewBounds);
}
void Enemy::onPlayerCollision(const Player& player) {
    if (player.isAttacking) {
        sf::FloatRect weaponCollision = player.getWeaponCollisionBounds();
        sf::FloatRect enemyBounds = this->getGlobalBounds();

        if (weaponCollision.intersects(enemyBounds)) {
            if (!wasHitThisAttack) {
                takeDamage(player.getAttackDamage(), player.meleeWeapon.damage);
                wasHitThisAttack = true;

                sf::Vector2f weaponCenter(
                    weaponCollision.left + weaponCollision.width / 2,
                    weaponCollision.top + weaponCollision.height / 2
                );

                sf::Vector2f direction = position - weaponCenter;
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0) {
                    direction /= length;
                }

                bounceVelocity = direction * player.pushForce;
                currentBounceTime = bounceDuration;
                setState(AnimationType::Bounce);
                setAnimation(AnimationType::Hit);
            }
        }
        else {
            // Сбрасываем флаг, если оружие больше не касается врага
            wasHitThisAttack = false;
        }
    }
}

void Enemy::drawPath(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!path.empty()) {
        sf::VertexArray lines(sf::LineStrip, path.size());
        for (size_t i = 0; i < path.size(); ++i) {
            lines[i].position = path[i];
            lines[i].color = sf::Color::Yellow;
        }
        target.draw(lines, states);
    }
}

bool Enemy::isPositionWalkable(const sf::Vector2f& pos) const {
    sf::Vector2f halfSize = collisionSize / 2.0f;

    std::vector<sf::Vector2f> points = {
        sf::Vector2f(pos.x - halfSize.x, pos.y - halfSize.y),
        sf::Vector2f(pos.x + halfSize.x, pos.y - halfSize.y),
        sf::Vector2f(pos.x - halfSize.x, pos.y + halfSize.y),
        sf::Vector2f(pos.x + halfSize.x, pos.y + halfSize.y)
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

void Enemy::pushOtherEnemies(const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
    for (const auto& other : allEnemies) {
        if (other.get() == this) continue;

        float distance = calculateDistance(position, other->position);
        if (distance < MIN_PUSH_DISTANCE && distance > 0) {
            sf::Vector2f pushDir = position - other->position;
            pushDir /= distance;

            float force = PUSH_FORCE * (1.0f - distance / MIN_PUSH_DISTANCE);

            velocity += pushDir * force;
            other->applyPushForce(-pushDir * force);
        }
    }
}

float Enemy::calculateDistance(const sf::Vector2f& pos1, const sf::Vector2f& pos2) const {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool Enemy::isPathObstructed(const Player& player) const {
    sf::FloatRect enemyRect = shape.getGlobalBounds();

    std::vector<sf::Vector2f> checkPoints = {
        sf::Vector2f(enemyRect.left, enemyRect.top),
        sf::Vector2f(enemyRect.left + enemyRect.width, enemyRect.top),
        sf::Vector2f(enemyRect.left, enemyRect.top + enemyRect.height),
        sf::Vector2f(enemyRect.left + enemyRect.width, enemyRect.top + enemyRect.height)
    };

    int obstructedCount = 1;

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

    return obstructedCount > 1;
}

void Enemy::calculatePath(const Player& player) {
    sf::Vector2f playerPosition = player.getPosition();
    sf::Vector2f direction = position - playerPosition;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0) {
        direction /= length;
    }
    playerPosition -= direction * 5.0f;

    path = findPath(position, playerPosition, level);
    pathIndex = 0;
}

void Enemy::moveTowardsPlayer(float deltaTime, const Player& player) {
    sf::Vector2f direction = player.getPosition() - position;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (length > 0) {
        direction /= length;
    }

    velocity = direction * speed;
    Entity::update(deltaTime);
}

bool Enemy::isTileWalkable(int x, int y, const Level& level) const {
    if (x < 0 || x >= level.width || y < 0 || y >= level.height) {
        return false;
    }

    if (level.layers.empty()) {
        return false;
    }

    int tileGID = level.layers[0].tileGID[x + y * level.width];

    for (const auto& object : level.getObjects()) {
        if (object.type == "solid" &&
            x * level.tileWidth >= object.rect.left && x * level.tileWidth < object.rect.left + object.rect.width &&
            y * level.tileHeight >= object.rect.top && y * level.tileHeight < object.rect.top + object.rect.height) {
            return false;
        }
    }

    return true;
}

void Enemy::updateCollisionRect() {
    rect.left = position.x - collisionSize.x / 2;
    rect.top = position.y - collisionSize.y / 2;
    rect.width = collisionSize.x;
    rect.height = collisionSize.y;
}

void Enemy::addDamageText(int damage, bool isCritical) {
    damageTexts.emplace_back(position.x, position.y - 30, damage, isCritical);
}

void Enemy::updateDamageTexts(float deltaTime) {
    for (auto& text : damageTexts) {
        text.update(deltaTime);
    }

    // Удаляем завершенные тексты
    damageTexts.erase(
        std::remove_if(damageTexts.begin(), damageTexts.end(),
            [](const DamageText& text) { return text.isFinished(); }),
        damageTexts.end()
    );
}

void Enemy::drawDamageTexts(sf::RenderWindow& window) {
    for (const auto& text : damageTexts) {
        text.draw(window);
    }
}

void Enemy::draw(sf::RenderWindow& window) {
    // Не рисуем врага, если анимация смерти завершена
    if (isDead() && deathAnimationFinished) {
        return;
    }

    // Рисуем спрайт (если текстура загружена)
    if (texture.getSize().x > 0) {
        window.draw(sprite);
    }
    else {
        // Дебаг-прямоугольник, если текстуры нет
        sf::RectangleShape debugRect(visualSize);
        debugRect.setPosition(position);
        debugRect.setOrigin(visualSize.x / 2, visualSize.y / 2);
        debugRect.setFillColor(sf::Color::Green);
        window.draw(debugRect);
    }

    // Рисуем здоровье только если враг не мёртв
    if (!isDead()) {
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

        window.draw(healthBar);
        window.draw(healthBarFill);

        // Дебаг-хитбокс
      sf::RectangleShape hitbox(collisionSize);
      hitbox.setPosition(position);
      hitbox.setOrigin(collisionSize.x / 2, collisionSize.y / 2);
      hitbox.setFillColor(sf::Color::Transparent);
      hitbox.setOutlineThickness(1);
      window.draw(hitbox);

        if (currentState == AnimationType::Attack && currentFrame >= framesPerAnimation / 2) {
            sf::RectangleShape attackArea(attackAreaSize);
            attackArea.setPosition(position);
            attackArea.setOrigin(isFacingRight ?
                sf::Vector2f(-collisionSize.x / 2, attackAreaSize.y / 2) :
                sf::Vector2f(collisionSize.x / 2 + attackAreaSize.x, attackAreaSize.y / 2));
            attackArea.setFillColor(sf::Color(255, 0, 0, 100));
            window.draw(attackArea);
        }
    }

    // Дебаг-путь (если нужно)
    if (!path.empty()) {
        sf::VertexArray lines(sf::LinesStrip, path.size());
        for (size_t i = 0; i < path.size(); ++i) {
            lines[i].position = path[i];
            lines[i].color = (i == pathIndex) ? sf::Color::Red : sf::Color::Green;
        }
        window.draw(lines);

        sf::CircleShape targetMarker(3);
        targetMarker.setFillColor(sf::Color::Red);
        targetMarker.setOrigin(3, 1.5);
        targetMarker.setPosition(path[pathIndex]);
        window.draw(targetMarker);
    }

    drawDamageTexts(window);
}