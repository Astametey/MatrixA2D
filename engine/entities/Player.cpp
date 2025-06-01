#include "Player.h"

Player::Player(float x, float y, float collisionWidth, float collisionHeight, float visualWidth, float visualHeight)
    : Entity(x, y, collisionWidth, collisionHeight)
{

    // ��������� �����
    
    attackCooldown = 0.3f;
    attackDuration = 0.30f; // ��������� ������������ ����� ��� ����� ������� ��������
    attackStartAngle = 180.f; // ���� ���������� �����
    attackEndAngle = -10.f;   // ���� ����������� ����� �����

    visualSize = sf::Vector2f(visualWidth, visualHeight);
    shape.setSize(visualSize);

    setHair.x = 1;
    setHair.y = 0;
    setEye.x = 0;
    setEye.y = 1; 


    if (!texture.loadFromFile("resources/textures/entities/player/player_body.png")) {
        shape.setFillColor(sf::Color::Green);
    }
    else {
        sprite.setTexture(texture);
        currentFrame = 0;
        frameWidth = 16;
        frameHeight = 32;
        sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
        shape.setFillColor(sf::Color::Transparent);
        frontHandSprite.setTexture(texture);
        backHandSprite.setTexture(texture);
        frontHandSprite.setTextureRect(sf::IntRect(64, 0, frameWidth, frameHeight));
        backHandSprite.setTextureRect(sf::IntRect(64, 0, frameWidth, frameHeight));
        sprite.setOrigin(frameWidth / 2, frameHeight / 2);

        if (!textureEye.loadFromFile("resources/textures/entities/player/player_eyes.png")) {
            shape.setFillColor(sf::Color::Green);
        }
        spriteEye.setTexture(textureEye);  
        spriteEye.setTextureRect(sf::IntRect(setEye.x * 16, setEye.y * 16, 16, 16));
        spriteEye.setOrigin(frameWidth / 2, frameHeight / 2);

        if (!textureHair.loadFromFile("resources/textures/entities/player/player_hair.png")) {
            shape.setFillColor(sf::Color::Green);
        }
        spriteHair.setTexture(textureHair);
        spriteHair.setTextureRect(sf::IntRect(setHair.x * 16, setHair.y * 16, 16, 32));
        spriteHair.setOrigin(frameWidth / 2, frameHeight / 2);

        shape.setOrigin(sprite.getOrigin().x, 16);
    }

    // ������������� origin ������� ��������� � �����
    sprite.setOrigin(frameWidth / 2.0f, frameHeight / 2.0f);
    frontHandSprite.setOrigin(6, 17);
    backHandSprite.setOrigin(6, 17);
    frontHandOffset.x = -3;//offset hands
    backHandOffset.x = -3;
    frontHandOffset.y = 2;
    backHandOffset.y = 2;

    speed = 100.0f;
    //level = lvl;

    animationClock.restart();
    isFacingRight = true;

}


void Player::update(float deltaTime) {
    // ��������� ����������� � ������� ����� ����� ���������

    // �������� ������� � ����������� �� ����������� � �������
    if (mousePos.x > position.x)
    {
        isFacingRight = 1;
    }
    else
    {
        isFacingRight = 0;
    }
    
    // ��������� �������
    for (auto& projectile : projectiles) {
        projectile.update(deltaTime);  // ������ ���������� ����� update �������
    }

    // ������� "�������" �������
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const ProjectileData& p) { return !p.isAlive(); }),
        projectiles.end()
    );

    // ��������� ������� ��������
    if (currentShootCooldown > 0.f) {
        currentShootCooldown -= deltaTime;
    }

    handleInput(deltaTime);
    Entity::update(deltaTime);
    updateAnimation();
    updateArmorPosition();

    // ��������� �����
    updateAttack(deltaTime);
    sprite.setPosition(position);
    spriteEye.setPosition(position);
    spriteEye.setScale(armorHead.sprite.getScale());
    spriteHair.setScale(armorHead.sprite.getScale());
    spriteHair.setPosition(position);

    updateWeaponPosition();
    


} 


void Player::setArmorHead(const Armor& armor) {
    armorHead = armor;
    armorHead.sprite.setTexture(armor.texture);
    armorHead.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
}
void Player::setArmorBody(const Armor& armor) {
    armorBody = armor;
    armorBody.sprite.setOrigin(frameWidth / 2, frameHeight / 2);
    armorBody.handSprite.setOrigin(frameWidth / 2, frameHeight / 2);


    // ������������� �������� ���� ���� ��� ��� ���������
    frontHandSprite.setTexture(armor.texture);
    backHandSprite.setTexture(armor.texture);
    frontHandSprite.setTextureRect(sf::IntRect(4 * frameWidth, 0, frameWidth, frameHeight));
    backHandSprite.setTextureRect(sf::IntRect(4 * frameWidth, 0, frameWidth, frameHeight));
}
void Player::setArmorLegs(const Armor& armor) {
    armorLegs = armor;
    armorLegs.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);

}
void Player::setArmorShoes(const Armor& armor) {
    armorShoes = armor;
    armorShoes.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);

}
void Player::setWeapon(const Weapon& weapon) {
    if (weapon.type == Weapon::Type::MELEE)
    {
        backHandSprite.setRotation(0);
        frontHandSprite.setRotation(0);
        std::cout << "Setting melee weapon: " << weapon.name << "\n";
        meleeWeapon = weapon;
        attackSpeed = meleeWeapon.attackSpeed;
        meleeWeapon.sprite.setOrigin(-1, -3); // ����� ����� (����������� ����������������)
        meleeWeapon.sprite.setTextureRect(sf::IntRect(0, 0, 32, 32)); // ������ ��������
        meleeWeapon.collisionOffset.x = -meleeWeapon.sprite.getOrigin().x + 8; 
        meleeWeapon.collisionOffset.y = -28;
        pushForce = meleeWeapon.pushForce;
    }
    else
    {
        std::cout << "Setting ranged weapon: " << weapon.name << "\n";
        rangedWeapon = weapon;
        rangedWeapon.sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
        rangedWeapon.sprite.setOrigin(14, 12);
    }
    ifEqupWeapon = true;
    
}

void Player::unqArmorHead()
{
    armorHead.name = "";
    armorHead.defense = 0;
}
void Player::unqArmorBody() {
    armorBody.name = "";
    armorBody.defense = 0;

    frontHandSprite.setTexture(texture);
    backHandSprite.setTexture(texture);
    frontHandSprite.setTextureRect(sf::IntRect(4 * frameWidth, 0, frameWidth, frameHeight));
    backHandSprite.setTextureRect(sf::IntRect(4 * frameWidth, 0, frameWidth, frameHeight));
}
void Player::unqArmorLegs() {
    armorLegs.name = "";
    armorLegs.defense = 0;

}
void Player::unqArmorShoes() {
    armorShoes.name = "";
    armorShoes.defense = 0;
}
void Player::unqWeapon() {
    if (!meleeWeapon.name.empty()) {
        meleeWeapon = Weapon();
        pushForce = 100.0f;
    }
    if (!rangedWeapon.name.empty()) {
        rangedWeapon = Weapon();
    }
    backHandSprite.setRotation(0);
    frontHandSprite.setRotation(0);
    ifEqupWeapon = false;
}


void Player::shoot() {
    if (currentShootCooldown > 0.f || rangedWeapon.texture.getSize().x == 0) return;

    // ��������� ����������� �������� (� �������)
    sf::Vector2f direction = mousePos - position;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.f) {
        direction /= length; // ����������� ������

        // ������� ������ � ����������� ��������� ��������, ����� �� �� ����� ������� � ������
        sf::Vector2f startPos;
        if (isFacingRight)
        {
            startPos = position + direction * 8.0f;
        }
        else
        {
            startPos = position + direction * 8.0f;
        }

        projectiles.push_back({
            startPos,
            direction * rangedWeapon.projectileSpeed, // ���������� �������� �� ������
            rangedWeapon.projectileLifetime, // ����� ����� �� ������
            getAttackDamage(false) // ���� �������� ������
            });

        currentShootCooldown = rangedWeapon.attackCooldown * 2; // ���������� ������� �� ������
    }
}
void Player::startAttack(bool autoAttack) {
    if (currentCooldown > 0.f || isAttacking) return;

    isAttacking = true;
    isAutoAttacking = autoAttack;
    currentAttackTime = 0.f;

    // ������������� ��������� ���� ����� � ����������� �� �����������
    if (isFacingRight) {
        attackAngle = attackStartAngle;
    }
    else {
        attackAngle = -attackStartAngle;
    }

    frontHandSprite.setRotation(attackAngle);

    // ���������� �������� �������� � ��������� ���������
    if (meleeWeapon.texture.getSize().x > 0) {
    }
}

void Player::stopAttack() {
    isAttacking = false;
    isAutoAttacking = false;
    attackAngle = 0;
    currentCooldown = attackCooldown;
    meleeWeapon.collisionOffset.y = -28;
    
}
void Player::updateAttack(float deltaTime) {
    if (currentCooldown > 0.f) {
        currentCooldown -= deltaTime;
    }
    
    if (isAttacking) {
        currentAttackTime += deltaTime;
        float progress = currentAttackTime / attackDuration;

        if (progress >= 1.f) {
            stopAttack();
            if (isAutoAttacking && autoAttackClock.getElapsedTime().asSeconds() >= autoAttackInterval) {
                startAttack(true);
                autoAttackClock.restart();
            }
        }
        else {
            if (isFacingRight) {
                attackAngle = -attackStartAngle + progress * (-attackEndAngle - -attackStartAngle);
            }
            else {
                attackAngle = attackStartAngle + progress * (attackEndAngle - (attackStartAngle));
            }
            frontHandSprite.setRotation(attackAngle);

        }
        meleeWeapon.collisionOffset.y += deltaTime * attackSpeed;
       
    }

    updateWeaponPosition();
}

bool Player::isMeleeEquipped() const{
    return meleeWeapon.texture.getSize().x > 0;
}

float Player::getDefense() const {
    float defense = 0.0f;
    if (!armorHead.texture.getSize().x == 0) defense += armorHead.defense;
    if (!armorBody.texture.getSize().x == 0) defense += armorBody.defense;
    if (!armorLegs.texture.getSize().x == 0) defense += armorLegs.defense;
    if (!armorShoes.texture.getSize().x == 0) defense += armorShoes.defense;
    return defense;
}
float Player::getSpeed() const {
    return speed; // ��� ����� �������� ������������ �� ����������
}
float Player::getBaseDamage(bool useMelee = true) const {
    const Weapon& currentWeapon = useMelee ? meleeWeapon : rangedWeapon;
    if (currentWeapon.texture.getSize().x > 0) {
        return currentWeapon.damage;
    }
    return 1.0f; // ������� ���� ��� ������
}
float Player::getAttackDamage(bool useMelee) const {
    float baseDamage = getBaseDamage(useMelee);

    // �������� �� ����������� ����
    if (static_cast<float>(rand()) / RAND_MAX < getCriticalChance(useMelee)) {
        return baseDamage * getCriticalMultiplier(useMelee);
    }

    return baseDamage;
}
sf::FloatRect Player::getWeaponCollisionBounds() const {
    if (isMeleeEquipped()) {
        return meleeWeapon.getCollisionBounds(isFacingRight);
    }
    else if (rangedWeapon.name == "")
    {
        // �������� ��� ����� ��� ������
        sf::FloatRect bounds = frontHandSprite.getGlobalBounds();

        if (isFacingRight) {
            bounds.left = position.x + 6;
            bounds.top = position.y;
            bounds.width = 5.0f;
            bounds.height = 5.0f;
        }
        else {
            bounds.left = position.x - 12;
            bounds.top = position.y;
            bounds.width = 5.0f;
            bounds.height = 5.0f;
        }
        return bounds;
    }
    
}

float Player::getCriticalChance(bool useMelee = true) const {
    const Weapon& currentWeapon = useMelee ? meleeWeapon : rangedWeapon;
    if (currentWeapon.texture.getSize().x > 0) {
        return currentWeapon.criticalChance;
    }
    return 0.0f; // ������� ���� ��� ������
}

float Player::getCriticalMultiplier(bool useMelee = true) const {
    const Weapon& currentWeapon = useMelee ? meleeWeapon : rangedWeapon;
    if (currentWeapon.texture.getSize().x > 0) {
        return currentWeapon.criticalMultiplier;
    }
    return 1.0f; // ������� ��������� ��� ������
}

void Player::handleInput(float deltaTime)
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

    // ��������� �����
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !isAutoAttacking)
    {
        startAttack();
        shoot();
    }

    // ���������/���������� ��������� (��������, �� ������� ������� R)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    {
        if (!isAutoAttacking && !isAttacking && currentCooldown <= 0.f)
        {
            startAttack(true);
        }
    }
    else if (isAutoAttacking)
    {
        stopAttack();
    }

    // Normalize velocity if moving diagonally
    if (velocity.x != 0 && velocity.y != 0)
    {
        velocity /= std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        velocity *= speed;
    }

    armorHead.sprite.setTextureRect(sf::IntRect(sprite.getTextureRect().left, armorHead.set_rect_xy.y, sprite.getTextureRect().width, sprite.getTextureRect().height));
    armorBody.sprite.setTextureRect(sf::IntRect(sprite.getTextureRect().left, armorBody.set_rect_xy.y, sprite.getTextureRect().width, sprite.getTextureRect().height));
    armorLegs.sprite.setTextureRect(sf::IntRect(sprite.getTextureRect().left, armorLegs.set_rect_xy.y, sprite.getTextureRect().width, sprite.getTextureRect().height));
    armorShoes.sprite.setTextureRect(sf::IntRect(sprite.getTextureRect().left, armorShoes.set_rect_xy.y, sprite.getTextureRect().width, sprite.getTextureRect().height));
}

// Animation update
void Player::updateAnimation() {
    if (velocity.x != 0 || velocity.y != 0) {
        if (animationClock.getElapsedTime().asMilliseconds() > 100) {
            currentFrame = (currentFrame + 1) % 4;
            sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight));
            animationClock.restart();
        }
    }
    else {
        currentFrame = 1;
        sprite.setTextureRect(sf::IntRect(frameWidth, 0, frameWidth, frameHeight));
    }

    if (isMeleeEquipped()) {
        meleeWeapon.setCollision(16, -16, 15, 8, isFacingRight);
    }

    // ������������� ������� ��� ��������� �������
    sprite.setScale(isFacingRight ? 1.f : -1.f, 1.f);

    if (rangedWeapon.texture.getSize().x == 0) {
        // ��������� ������� ��� � ������ �����������
        sf::Vector2f frontHandPos, backHandPos;
        if (isFacingRight) {
            frontHandPos = position + frontHandOffset;
            backHandPos = position + backHandOffset;
        }
        else {
            frontHandPos = position + sf::Vector2f(-frontHandOffset.x, frontHandOffset.y);
            backHandPos = position + sf::Vector2f(-backHandOffset.x, backHandOffset.y);
        }

        // ������������� ������� � ������� ���
        frontHandSprite.setScale(sprite.getScale());
        backHandSprite.setScale(sprite.getScale());
        frontHandSprite.setPosition(frontHandPos);
        backHandSprite.setPosition(backHandPos);

        // ���� �������� �������
        if (isAttacking) {
            float attackProgress = currentAttackTime / attackDuration;

            // ��������� ������ ���� �����
            float backArmOffset = 2.f * sin(attackProgress * 3.14159f);
            if (isFacingRight) {
                backHandSprite.setPosition(backHandPos.x - backArmOffset, backHandPos.y);
            }
            else {
                backHandSprite.setPosition(backHandPos.x + backArmOffset, backHandPos.y);
            }

            // �������� ������ ���� �� ����� �����
            float backArmRotation = 0.f;
            if (attackProgress < 0.5f) {
                backArmRotation = -15.f * (1.f - 2.f * attackProgress);
            }
            else {
                backArmRotation = -15.f * (2.f * attackProgress - 1.f);
            }

            backHandSprite.setRotation(isFacingRight ? backArmRotation : -backArmRotation);
        }
        else { 
            backHandSprite.setRotation(0.f);
        }
    }

    if (currentShootCooldown > 0.f)
    {
        rangedWeapon.sprite.setTextureRect(sf::IntRect(02, 0, 32, 32));
    }
    else
    {
        rangedWeapon.sprite.setTextureRect(sf::IntRect(32, 0, 32, 32));
    };
    
}
void Player::updateArmorPosition() {
    // ������������� ������� � ������� ���� ������ �����
    sf::Vector2f armorScale = isFacingRight ? sf::Vector2f(1.f, 1.f) : sf::Vector2f(-1.f, 1.f);

    armorHead.setPosition(position);
    armorBody.setPosition(position);
    armorLegs.setPosition(position);
    armorShoes.setPosition(position);

    armorHead.sprite.setScale(armorScale);
    armorBody.sprite.setScale(armorScale);
    armorLegs.sprite.setScale(armorScale);
    armorShoes.sprite.setScale(armorScale);

    // ����������� ��������� ��� ���� �� �����
    if (armorBody.texture.getSize().x > 0) {
        armorBody.handSprite.setPosition(frontHandSprite.getPosition());
        armorBody.handSprite.setRotation(frontHandSprite.getRotation());
        armorBody.handSprite.setScale(armorScale);
    }
}
void Player::updateWeaponPosition() {
    if (meleeWeapon.texture.getSize().x > 0) {
        // ��������� ���������������� ������
        float weaponDistance = 14.f; // ���������� �� ���� �� ������ ����
        float gripOffset = 4.f; // �������� ��� ����������� �����
        // ����������� ���� � �������
        float angleRad = frontHandSprite.getRotation() * 3.14159265f / 180.f;

        // ��������� �������� ������ � ������ ���� ����� � �����������
        sf::Vector2f weaponOffset;
        if (isFacingRight) {
            weaponOffset = sf::Vector2f(
                weaponDistance * cos(angleRad) + gripOffset * sin(angleRad),
                weaponDistance * sin(angleRad) - gripOffset * cos(angleRad)
            );
        }
        else {
            weaponOffset = sf::Vector2f(
                -weaponDistance * cos(angleRad) + gripOffset * sin(angleRad),
                weaponDistance * sin(angleRad) - gripOffset * cos(angleRad)
            );
        }

        // ������������� ������� � ������� ������
        meleeWeapon.sprite.setPosition(frontHandSprite.getPosition());
        meleeWeapon.sprite.setScale(frontHandSprite.getScale());
        meleeWeapon.sprite.setRotation(frontHandSprite.getRotation());
    }

    // ��� �������� ������ (���� � �.�.)
    if (rangedWeapon.texture.getSize().x > 0) {
        float angleToMouse = getAngleToMouse();
        // ������������� ������� ��� � ������� �������
        if (isFacingRight)
        {
            frontHandSprite.setRotation(angleToMouse - 70);
            backHandSprite.setRotation(angleToMouse - 70);
        }
        else
        {
            
            frontHandSprite.setRotation(angleToMouse - 110);
            backHandSprite.setRotation(angleToMouse - 110);
        }
        
        

        // ��������� ������� ��� � ������ �����������
        sf::Vector2f frontHandPos, backHandPos;
        if (isFacingRight) {
            frontHandPos = position + frontHandOffset;
            backHandPos = position + backHandOffset;
        }
        else {
            frontHandPos = position + sf::Vector2f(-frontHandOffset.x, frontHandOffset.y);
            backHandPos = position + sf::Vector2f(-backHandOffset.x, backHandOffset.y);
        }
        
        // ������������� ������� � ������� ���
        frontHandSprite.setScale(sprite.getScale());
        backHandSprite.setScale(sprite.getScale());
        frontHandSprite.setPosition(frontHandPos);
        backHandSprite.setPosition(backHandPos);


        rangedWeapon.sprite.setPosition(frontHandSprite.getPosition().x, frontHandSprite.getPosition().y);
        rangedWeapon.sprite.setScale(frontHandSprite.getScale());
        
        rangedWeapon.sprite.setRotation(frontHandSprite.getRotation());
        
        
    }
}

bool Player::checkCollisionWithLevel(const Level& level) {
    bool collisionOccurred = false;

    // �������� ������� ������� ������
    sf::FloatRect playerBounds = getGlobalBounds();

    // ��������� �������� �� ����� ��������� ������
    for (const auto& object : level.getObjects()) {
        if (object.type == "solid") {
            sf::FloatRect obstacleBounds(
                static_cast<float>(object.rect.left),
                static_cast<float>(object.rect.top),
                static_cast<float>(object.rect.width),
                static_cast<float>(object.rect.height)
            );

            if (playerBounds.intersects(obstacleBounds)) {
                resolveCollision(obstacleBounds);
                collisionOccurred = true;
            }
        }
    }

    return collisionOccurred;
}
void Player::resolveCollision(const sf::FloatRect& obstacle) {
    sf::FloatRect playerBounds = getGlobalBounds();

    // ��������� ������� ������������� � ������ �������
    float overlapLeft = playerBounds.left + playerBounds.width - obstacle.left;
    float overlapRight = obstacle.left + obstacle.width - playerBounds.left;
    float overlapTop = playerBounds.top + playerBounds.height - obstacle.top;
    float overlapBottom = obstacle.top + obstacle.height - playerBounds.top;

    // ������� ����������� ����������
    bool fromLeft = overlapLeft < overlapRight;
    bool fromTop = overlapTop < overlapBottom;

    float minOverlapX = fromLeft ? overlapLeft : overlapRight;
    float minOverlapY = fromTop ? overlapTop : overlapBottom;

    // ������ �������� �� ��� � ���������� �����������
    if (minOverlapX < minOverlapY) {
        // �������� �� X
        if (fromLeft) {
            position.x -= minOverlapX;
            velocity.x = 0; // ������������� �������� � ���� �����������
        }
        else {
            position.x += minOverlapX;
            velocity.x = 0; // ������������� �������� � ���� �����������
        }
    }
    else {
        // �������� �� Y
        if (fromTop) {
            position.y -= minOverlapY;
            velocity.y = 0; // ������������� �������� � ���� �����������
        }
        else {
            position.y += minOverlapY;
            velocity.y = 0; // ������������� �������� � ���� �����������
        }
    }
}


float Player::getAngleToMouse() const {
    sf::Vector2f direction = mousePos - position;
    return std::atan2(direction.y, direction.x) * 180.f / 3.14159265f;
}
void Player::pushBack(const sf::Vector2f& direction, float force) {
    // ����������� ����������� (���� ��� �� �������)
    if (direction.x != 0 || direction.y != 0) {
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        sf::Vector2f normalizedDir = direction / length;

        // ��������� ���� ������������
        velocity += normalizedDir * force;
    }
}


void Player::draw(sf::RenderWindow& window)
{
    if (isAttacking) {
        // ������������ �������� ������ (������ ��� �������)
        sf::FloatRect weaponCollision = getWeaponCollisionBounds();
        sf::RectangleShape collisionRect(sf::Vector2f(weaponCollision.width, weaponCollision.height));
        collisionRect.setPosition(weaponCollision.left, weaponCollision.top);
        collisionRect.setFillColor(sf::Color(255, 0, 0, 100));
        window.draw(collisionRect);
    }
    // ������ ������ ������
    if (texture.getSize().x > 0) {
        if (isFacingRight)
        {
            window.draw(backHandSprite);//back
            window.draw(sprite);//body
            window.draw(spriteEye);//eyes
            if (armorHead.name != "")
            {
                window.draw(armorHead.sprite);//head
            }
            else
            {
                window.draw(spriteHair);//hair
            }

            if (armorLegs.name != "")
                window.draw(armorLegs.sprite);//legs
            if (armorShoes.name != "")
                window.draw(armorShoes.sprite);//shoes
            if (armorBody.name != "")
                window.draw(armorBody.sprite);//body armor
            if (meleeWeapon.name != "")
                window.draw(meleeWeapon.sprite);//sword
            if (rangedWeapon.name != "")
                window.draw(rangedWeapon.sprite);//bow
            window.draw(frontHandSprite);//front arm}
        }
        else
        {
            if (rangedWeapon.name != "")
                window.draw(rangedWeapon.sprite);//bow
            if (meleeWeapon.name != "")
                window.draw(meleeWeapon.sprite);//sword
            window.draw(frontHandSprite);//front arm
            window.draw(sprite);//body
            window.draw(spriteEye);//eyes
            if (armorHead.name != "")
            {
                window.draw(armorHead.sprite);//head
            }
            else
            {
                window.draw(spriteHair);//hair
            }
            if (armorLegs.name != "")
                window.draw(armorLegs.sprite);//legs
            if (armorShoes.name != "")
                window.draw(armorShoes.sprite);//shoes
            if (armorBody.name != "")
                window.draw(armorBody.sprite);//body armor
            window.draw(backHandSprite);//back arm
            
        }
    }
    else {
        Entity::draw(window);
    }
    drawDebugCollision(window);

    // ������ ������� (������������� ������� �����)
    for (const auto& projectile : projectiles) {
        if (projectile.isAlive()) {
            window.draw(projectile.shape);
        }
    }

}
