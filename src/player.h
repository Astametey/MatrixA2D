#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "entity.h"


class Player;

// Структура для хранения информации о броне
struct Armor {
    std::string name;
    float defense;
    sf::Texture texture;
    sf::Sprite sprite;  // Спрайт для тела
    sf::Sprite handSprite;  // Спрайт для руки (5й кадр)
    sf::Vector2f offset;
    sf::Vector2f set_rect_xy;

    Armor() : name(""), defense(0.0f), offset(0, 0) {}

    bool loadTexture(const std::string& filename) {
        if (!texture.loadFromFile(filename)) {
            std::cerr << "Failed to load armor texture: " << filename << std::endl;
            return false;
        }
        sprite.setTexture(texture);
        handSprite.setTexture(texture);

        // Устанавливаем текстуру руки (5й кадр) один раз при загрузке
        handSprite.setTextureRect(sf::IntRect(4 * 16, 0, 16, 32)); // Предполагаем frameWidth=16

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
    void set_armor_type(int x, int y) {
        sprite.setTextureRect(sf::IntRect(x * 16, y * 32, 16, 16));
    }

    sf::Sprite HandGetTexture()
    {
        return handSprite;
    }


};

// Структура для хранения информации об оружии
struct Weapon {
    enum class Type {
        MELEE,
        RANGED
    };

    std::string name;
    Type type;
    float damage;
    float attackSpeed;
    float criticalChance;  // Шанс критического удара
    float criticalMultiplier; // Множитель критического урона
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Vector2f meleeWeaponOffset;

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
        sprite.setPosition((position + meleeWeaponOffset));
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    virtual void attack(Player* target) {
        std::cout << "Generic weapon attack!" << std::endl;
    }

};

// Класс игрока, наследуется от Entity
class Player : public Entity
{
public:
    // Добавляем новые члены класса
    bool isAttacking = false;
    sf::Clock attackCooldown;
    float attackDuration = 0.5f; // Длительность атаки в секундах
    sf::Vector2f playerToMouse;
    sf::Vector2f mousePos;

    Armor armorHead;
    Armor armorBody;
    Armor armorLegs;
    Armor armorShoes;
    Weapon meleeWeapon;
    Weapon rangedWeapon;

    //Добавляем спрайты для рук персонажа(основные)
    sf::Sprite frontHandSprite;
    sf::Sprite backHandSprite;


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
            frameWidth = 16;
            frameHeight = 32;
            sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
            shape.setFillColor(sf::Color::Transparent);
            frontHandSprite.setTexture(texture);
            backHandSprite.setTexture(texture);
            frontHandSprite.setTextureRect(sf::IntRect(64, 0, frameWidth, frameHeight));
            backHandSprite.setTextureRect(sf::IntRect(64, 0, frameWidth, frameHeight));
            sprite.setOrigin(frameWidth / 2, frameHeight / 2);
        }

        // Загрузка текстуры атаки
        if (!attackTexture.loadFromFile("resources/Entities/attackArea.png")) {
            std::cerr << "Failed to load attack texture!" << std::endl;
        }
        else {
            attackSprite.setTexture(attackTexture);
            attackFrameWidth = attackTexture.getSize().x / 4; // Предполагаем 2 кадра
            attackFrameHeight = attackTexture.getSize().y;
            attackSprite.setTextureRect(sf::IntRect(0, 0, attackFrameWidth, attackFrameHeight));
            attackSprite.setOrigin(sprite.getOrigin()); // Используем тот же origin
        }


        // Устанавливаем origin спрайта персонажа в центр
        sprite.setOrigin(frameWidth / 2.0f, frameHeight / 2.0f);
        frontHandSprite.setOrigin(6, 17);
        backHandSprite.setOrigin(6, 17);

        speed = 100.0f;
        level = lvl;

        animationClock.restart();
        isFacingRight = true;

    } 

    ~Player() override {}

    void update(float deltaTime) override {
        // Обновляем направление к курсору перед всеми расчетами
        updateMouseDirection();

        // Разворот спрайта в зависимости от направления к курсору
        isFacingRight = playerToMouse.x > 0;

        handleInput(deltaTime);
        handleAttack();
        Entity::update(deltaTime);

        updateAnimation();
        sprite.setPosition(position);
        
        

        // Обновление позиций и поворотов
        updateArmorPosition();
        updateHandPosition();
        updateWeaponPosition();

        // Обновление анимации атаки
        updateAttackAnimation();
    }


    float getDefense() const {
        float defense = 0.0f;
        if (!armorHead.texture.getSize().x == 0) defense += armorHead.defense;
        if (!armorBody.texture.getSize().x == 0) defense += armorBody.defense;
        if (!armorLegs.texture.getSize().x == 0) defense += armorLegs.defense;
        if (!armorShoes.texture.getSize().x == 0) defense += armorShoes.defense;
        return defense;
    }
    float getSpeed() const {
        return speed; // или можно добавить модификаторы от экипировки
    }
    float getBaseDamage(bool useMelee = true) const {
        const Weapon& currentWeapon = useMelee ? meleeWeapon : rangedWeapon;
        if (currentWeapon.texture.getSize().x > 0) {
            return currentWeapon.damage;
        }
        return 1.0f; // Базовый урон без оружия
    }
    float getAttackDamage(bool useMelee = true) const {
        float baseDamage = getBaseDamage(useMelee);

        // Проверка на критический удар
        if (static_cast<float>(rand()) / RAND_MAX < getCriticalChance(useMelee)) {
            return baseDamage * getCriticalMultiplier(useMelee);
        }

        return baseDamage;
    }
    
    // Функция для получения шанса критического удара текущего оружия
    float getCriticalChance(bool useMelee = true) const {
        const Weapon& currentWeapon = useMelee ? meleeWeapon : rangedWeapon;
        if (currentWeapon.texture.getSize().x > 0) {
            return currentWeapon.criticalChance;
        }
        return 0.0f; // Базовый шанс без оружия
    }
    // Функция для получения множителя критического урона текущего оружия
    float getCriticalMultiplier(bool useMelee = true) const {
        const Weapon& currentWeapon = useMelee ? meleeWeapon : rangedWeapon;
        if (currentWeapon.texture.getSize().x > 0) {
            return currentWeapon.criticalMultiplier;
        }
        return 1.0f; // Базовый множитель без оружия
    }

    void handleAttack() {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            if (playerToMouse.x > 0) {
                isFacingRight = true;
            }
            else {
                isFacingRight = false;
            }

            if (!isAttacking) {
                // First attack in combo
                isAttacking = true;
                comboCount = 1;
                isUpwardAttack = false;
                isFirstAttackInCombo = true;
                attackCooldown.restart();
                comboClock.restart();
                performMeleeAttack();
            }
            else if (isAttacking && comboCount == 1 &&
                comboClock.getElapsedTime().asSeconds() < comboTimeout &&
                attackCooldown.getElapsedTime().asSeconds() > attackDuration * 0.5f) {
                // Second attack in combo
                comboCount = 2;
                comboClock.restart();
                attackCooldown.restart();
                isUpwardAttack = true;
                isFirstAttackInCombo = false;
                performComboAttack(isUpwardAttack);
            }

        }

        // Reset attack state after duration
        if (isAttacking && attackCooldown.getElapsedTime().asSeconds() > attackDuration) {
            isAttacking = false;
            if (comboClock.getElapsedTime().asSeconds() > comboTimeout) {
                comboCount = 0;
            }
        }
    }

    void performMeleeAttack()
    {
        

        //std::cout << "Performing melee attack!" << std::endl;
        // Сбрасываем анимацию
        showAttackAnimation = true;
        currentAttackFrame = 0;
        attackAnimationClock.restart();
        attackSprite.setTextureRect(sf::IntRect(0, 0, attackFrameWidth, attackFrameHeight));

        

        // Направление атаки - к курсору
        float angle = std::atan2(playerToMouse.y, playerToMouse.x) * 180.f / 3.14159265f;
        float attackDistance = 25.f;


        sf::Vector2f attackOffset(
            std::cos(angle * 3.14159265f / 180.f) * attackDistance,
            std::sin(angle * 3.14159265f / 180.f) * attackDistance
        );

        // Устанавливаем позицию анимации атаки (на расстоянии 20 пикселей от игрока в направлении курсора)
        
        // Определяем, куда смотрит игрок (по направлению к курсору)
        
        attackSprite.setPosition(meleeWeapon.sprite.getPosition());
        attackSprite.setScale(playerToMouse.x > 0 ? 1.f : -1.f, 1.f);
        isHandAttacking = true;
        attackHandClock.restart();
        // Анимация персонажa
        animationClock.restart();
    }

    void performRangedAttack()
    {
        std::cout << "Performing ranged attack!" << std::endl;

        // Здесь должна быть логика создания снаряда
        // Например, создание пули, которая летит в направлении курсора

        // Проигрываем анимацию атаки
        currentFrame = 3;
        sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight));
    }

    void performComboAttack(bool upward) {
        std::cout << "Combo attack #" << comboCount << (upward ? " (upward)" : " (downward)") << std::endl;

        showAttackAnimation = true;
        currentAttackFrame = 0;
        attackAnimationClock.restart();

        // Set attack animation position
        sf::Vector2f attackPosition = position;
        float attackOffsetX = isFacingRight ? 20.f : -20.f;
        float attackOffsetY = upward ? -15.f : 15.f;
        attackPosition.x += attackOffsetX;
        attackPosition.y += attackOffsetY;
        attackSprite.setPosition(attackPosition);
        attackSprite.setScale(isFacingRight ? 1.f : -1.f, 1.f);

        // Hand animation
        isHandAttacking = true;
        attackHandClock.restart();
    }

    // При установке брони добавьте смещения:
    void setArmorHead(const Armor& armor) {
        armorHead = armor;
        armorHead.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
    } 
    void setArmorBody(const Armor& armor) {
        armorBody = armor;
        armorBody.sprite.setOrigin(frameWidth / 2, frameHeight / 2);
        armorBody.handSprite.setOrigin(frameWidth / 2, frameHeight / 2);


        // Устанавливаем текстуру руки один раз при надевании
        frontHandSprite.setTexture(armor.texture);
        backHandSprite.setTexture(armor.texture);
        frontHandSprite.setTextureRect(sf::IntRect(4 * frameWidth, 0, frameWidth, frameHeight));
        backHandSprite.setTextureRect(sf::IntRect(4 * frameWidth, 0, frameWidth, frameHeight));
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
        meleeWeapon.sprite.setOrigin(6, 4); // Рукоять меча

    }
    void setRangedWeapon(const Weapon& weapon) {
        rangedWeapon = weapon;
        rangedWeapon.sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);

    }

    bool isMeleeEquipped() const {
        return meleeWeapon.texture.getSize().x > 0;
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

            attackSprite.setPosition(position);
        }

        

        armorBody.sprite.setTextureRect(sf::IntRect(sprite.getTextureRect().left, armorBody.set_rect_xy.y, sprite.getTextureRect().width, sprite.getTextureRect().height));
        armorLegs.sprite.setTextureRect(sf::IntRect(sprite.getTextureRect().left, armorLegs.set_rect_xy.y, sprite.getTextureRect().width, sprite.getTextureRect().height));
        armorShoes.sprite.setTextureRect(sf::IntRect(sprite.getTextureRect().left, armorShoes.set_rect_xy.y, sprite.getTextureRect().width, sprite.getTextureRect().height));
        
    }
    
    void updateMouseDirection() {
        playerToMouse = mousePos - position;

        // Нормализуем вектор направления
        float length = std::sqrt(playerToMouse.x * playerToMouse.x + playerToMouse.y * playerToMouse.y);
        if (length > 0) {
            playerToMouse /= length;
        }
    }
    void updateArmorPosition() {
        // Устанавливаем позицию и масштаб всех частей брони
        sf::Vector2f armorScale = isFacingRight ? sf::Vector2f(1.f, 1.f) : sf::Vector2f(-1.f, 1.f);

        armorHead.setPosition(position);
        armorBody.setPosition(position);
        armorLegs.setPosition(position);
        armorShoes.setPosition(position);

        armorHead.sprite.setScale(armorScale);
        armorBody.sprite.setScale(armorScale);
        armorLegs.sprite.setScale(armorScale);
        armorShoes.sprite.setScale(armorScale);

        // Специальная обработка для руки из брони
        if (armorBody.texture.getSize().x > 0) {
            armorBody.handSprite.setPosition(frontHandSprite.getPosition());
            armorBody.handSprite.setRotation(frontHandSprite.getRotation());
            armorBody.handSprite.setScale(armorScale);
        }
    }
    void updateAttackAnimation() {
        if (!showAttackAnimation) return;

        // Обновление кадров анимации атаки
        if (attackAnimationClock.getElapsedTime().asSeconds() > attackFrameDuration) {
            currentAttackFrame++;

            if (currentAttackFrame < 4) {
                // Устанавливаем следующий кадр анимации
                attackSprite.setTextureRect(sf::IntRect(
                    currentAttackFrame * attackFrameWidth,
                    0,
                    attackFrameWidth,
                    attackFrameHeight
                ));

                // Дополнительные эффекты при анимации
                if (currentAttackFrame == 2) { // Пиковый кадр атаки
                    // Можно добавить эффекты вроде увеличения области атаки
                    
                }

                attackAnimationClock.restart();
            }
            else {
                // Анимация завершена
                showAttackAnimation = false;
                
            }
        }

        // Плавное исчезновение анимации
        if (currentAttackFrame == 3) {
            float progress = attackAnimationClock.getElapsedTime().asSeconds() / attackFrameDuration;
            attackSprite.setColor(sf::Color(255, 255, 255, 255 - static_cast<sf::Uint8>(progress * 255)));
        }
        else {
            attackSprite.setColor(sf::Color::White);
        }
    }
    void updateHandPosition() {
        // Вычисляем угол к курсору в градусах
        float angleToMouse = std::atan2(playerToMouse.y, playerToMouse.x) * 180.f / 3.14159265f;

        // Основная рука всегда смотрит на курсор
        if (!isAttacking)
        {
            frontHandSprite.setRotation(angleToMouse - 60.f);
            attackSprite.setRotation(angleToMouse - 60.f);
        }
        

        // Задняя рука зеркально противоположна (с небольшим смещением для естественности)
        //backHandSprite.setRotation(angleToMouse + 15.f - 40.f);

        sf::Vector2f handOffset(
            -2 * sprite.getScale().x,
            2
        );

        frontHandSprite.setPosition(position + handOffset);
        backHandSprite.setPosition(position + handOffset); // Задняя рука чуть ближе

        // Анимация атаки добавляет дополнительное движение
        if (isHandAttacking) {
            updateAttackHandAnimation(angleToMouse);
        }
    }
    void updateWeaponPosition() {
        // Позиционируем меч относительно руки
        float weaponDistance = 14.f; // Расстояние от руки до центра меча
        float gripOffset = 4.f; // Смещение для правильного хвата

        float angleRad = frontHandSprite.getRotation() * 3.14159265f / 180.f;

        // Смещение с учетом хвата (чтобы меч был в руке)
        sf::Vector2f weaponOffset(
            std::cos(angleRad) * weaponDistance - std::sin(angleRad) * gripOffset,
            std::sin(angleRad) * weaponDistance + std::cos(angleRad) * gripOffset
        );

        meleeWeapon.sprite.setPosition(frontHandSprite.getPosition() + weaponOffset);
        meleeWeapon.sprite.setRotation(frontHandSprite.getRotation());

        // Отражаем спрайт оружия если смотрим влево
        bool facingRight = playerToMouse.x > 0;
        meleeWeapon.sprite.setScale(facingRight ? 1.f : -1.f, 1.f);

        // Для дальнего оружия (лука и т.д.)
        if (rangedWeapon.texture.getSize().x > 0) {
            rangedWeapon.sprite.setPosition(position);
            rangedWeapon.sprite.setRotation(frontHandSprite.getRotation());
            rangedWeapon.sprite.setScale(facingRight ? 1.f : -1.f, 1.f);
        }
    }
    void updateAttackHandAnimation(float baseAngle) {
        float attackProgress = attackHandClock.getElapsedTime().asSeconds() / (attackDuration * 0.6f);

        if (attackProgress < 1.0f) {
            // Плавная анимация замаха с эластичностью
            float swingAngle = 0.f;

            if (attackProgress < 0.3f) {
                // Фаза замаха назад
                swingAngle = -45.f * (attackProgress / 0.3f);
            }
            else if (attackProgress < 0.7f) {
                // Основная фаза удара
                swingAngle = -45.f + 135.f * ((attackProgress - 0.3f) / 0.4f);
            }
            else {
                // Фаза возврата с небольшим отскоком
                swingAngle = 90.f - 20.f * ((attackProgress - 0.7f) / 0.3f);
            }

            // Для второго удара комбо меняем направление
            if (comboCount == 2) {
                swingAngle = -swingAngle * 0.8f;
            }

            frontHandSprite.setRotation(baseAngle + swingAngle);

            // Эффект "размаха" - рука чуть дальше при атаке
            float pushDistance = 1.f * std::sin(attackProgress * 3.14159265f);
            float angleRad = frontHandSprite.getRotation() * 3.14159265f / 180.f;
            sf::Vector2f pushOffset(
                std::cos(angleRad) * pushDistance,
                std::sin(angleRad) * pushDistance
            );
            frontHandSprite.move(pushOffset);
        }
        else {
            // Анимация завершена
            isHandAttacking = false;
            frontHandSprite.setRotation(baseAngle);
        }
    }

    // Обновление анимации
    void updateAnimation()
    {
        // Если идет атака - не обновляем анимацию движения
        if (isAttacking) return;

        // Остальной код анимации движения...
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
        sprite.setScale(isFacingRight ? 1.f : -1.f, 1.f);
    }
    
    

protected:
    float speed;
    Level level;

    // Анимационные переменные
    sf::Texture texture;
    sf::Sprite sprite;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    sf::Clock animationClock;
    bool isFacingRight;
     
    sf::Texture attackTexture;
    sf::Sprite attackSprite;
    int attackFrameWidth;
    int attackFrameHeight;
    int currentAttackFrame = 0;
    bool showAttackAnimation = false;
    sf::Clock attackAnimationClock;
    float attackFrameDuration = 0.05f; // Длительность одного кадра анимации

    int comboCount = 0;
    sf::Clock comboClock;
    float comboTimeout = 0.3f; // Время между ударами для комбо
    bool isUpwardAttack = false;


    // Переменные для управления руками
    float handSwingTime = 0.0f;
    float handSwingSpeed = 5.0f;
    float handSwingAmount = 15.0f;
    float frontHandOffsetX = -3.0f;
    float backHandOffsetX = 2.0f;
    float handOffsetY = 2.0f;
    float handRotation = 0.0f;
    bool isHandAttacking = false;
    sf::Clock attackHandClock;

    bool isFirstAttackInCombo = true; // Флаг для первого удара в комбо
    float comboAttackStartRotation = 0.0f; // Начальный угол для анимации комбо
    float comboAttackTargetRotation = 90.0f; // Целевой угол для анимации комбо

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {

        // Отрисовка rect для отладки
        //sf::RectangleShape debugRect(sf::Vector2f(rect.width, rect.height));
        //debugRect.setPosition(rect.left, rect.top);
        //debugRect.setFillColor(sf::Color::Transparent);
        //debugRect.setOutlineColor(sf::Color::Green);
        //debugRect.setOutlineThickness(1);
        //target.draw(debugRect, states);


        
        // Рисуем спрайт игрока
        if (texture.getSize().x > 0) {
            target.draw(backHandSprite, states);//Задняя рука
            target.draw(sprite, states);//Тело база
            target.draw(armorBody.sprite, states);//Броня
            target.draw(armorHead.sprite, states);//Голова
            target.draw(armorLegs.sprite, states);//Ноги
            target.draw(armorShoes.sprite, states);//Ступни
            target.draw(meleeWeapon.sprite, states);//Меч
            target.draw(rangedWeapon.sprite, states);//Лук
            target.draw(frontHandSprite, states);//Передняя рука
        } 
        else {
            Entity::draw(target, states);
        }

        // Рисуем анимацию атаки, если она активна
        if (showAttackAnimation) {
            target.draw(attackSprite, states);
        }

        // Рисуем область атаки для отладки (можно убрать в финальной версии)
        if (isAttacking) {
            
        }

    }
};

#endif  