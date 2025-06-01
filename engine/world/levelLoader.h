#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include <string>
#include <vector>
#include <map>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include "tinyxml/tinyxml.h"
#include "Camera.h"

struct Layer {
    std::vector<int> tileGID; // Вектор идентификаторов тайлов
    std::string name;       // Имя слоя
    bool alwaysOnTop;       // Всегда рисуется поверх игрока
    int zOrder;             // Порядок отрисовки (чем больше, тем выше)
};

class Object {
public:
    int GetPropertyInt(std::string name) const;
    float GetPropertyFloat(std::string name) const;
    std::string GetPropertyString(std::string name) const;
    std::string name;
    std::string type;
    bool isSolid = false;   
    bool canOverlap = false;
    bool alwaysBehind = false; // Всегда рисуется под игроком
    sf::Rect <float> rect;
    std::map <std::string, std::string> properties;//All properties of the object. Values are stored as strings and mapped by strings(names provided in editor).
    int layer = 0; // Номер слоя объекта
    void setRect(float width, float height, float x, float y);
};


class Level {
public:
    int tileWidth, tileHeight;
    sf::Image tileMap_image;
    sf::Texture tileMap_texture;
    sf::Sprite tileMap_sprite;
    bool LoadFromFile(std::string filename, std::string tileset_path);//возвращает false если не получилось загрузить
    void LoadObjects(TiXmlElement* map);
    std::vector<sf::IntRect> num_tile;
    sf::IntRect GetTileSize;//получаем размер тайла
    void DrawTiles(const sf::FloatRect& viewBounds, sf::RenderWindow* window);
    void DrawLayer(int layerIndex, const sf::FloatRect& viewBounds, sf::RenderWindow* window);
    void DrawLayerByName(const std::string& layerName, const sf::FloatRect& viewBounds, sf::RenderWindow* window);

    void DrawObjectsBehindPlayer(const sf::FloatRect& viewBounds, sf::RenderWindow* window);
    void DrawObjectsInFrontOfPlayer(const sf::FloatRect& viewBounds, const sf::Vector2f& playerPos, sf::RenderWindow* window);
    void DrawObject(const Object& object, const sf::FloatRect& viewBounds, sf::RenderWindow* window);
    Level();
    void L_Free();
    Object GetObject(std::string name);
    std::vector<Object*> GetObjectsForRendering(const sf::FloatRect& viewBounds, const sf::Vector2f& playerPos);
    bool isPositionWalkable(const sf::Vector2f& position) const;

    std::vector<Layer> layers;
    std::vector<int> objlayers;
    std::vector<Object> getObjects() const { return objects; } // геттер для доступа к вектору объектов

    int width, height;//в tmx файле width height в начале,затем размер тайла

private:
    int firstTileID;//получаем айди первого тайла
    std::vector <sf::Rect <int> > solidObjects;
    //This stores all objects (including 'solid' types)
    std::vector <Object> objects;


};

#endif // LEVELLOADER_H