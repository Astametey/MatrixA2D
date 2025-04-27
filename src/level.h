#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <vector>
#include <map>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include "tinyxml/tinyxml.h"
#include "view.h"


class Object {
public:
	int GetPropertyInt(std::string name);
	float GetPropertyFloat(std::string name);
	std::string GetPropertyString(std::string name);
	std::string name;
	std::string type;
	sf::Rect <float> rect;
	std::map <std::string, std::string> properties;//All properties of the object. Values are stored as strings and mapped by strings(names provided in editor).
	int layer; // Номер слоя объекта
};

int Object::GetPropertyInt(std::string name)
{
	int i;
	i = atoi(properties[name].c_str());

	return i;
}

float Object::GetPropertyFloat(std::string name)
{
	float f;
	f = strtod(properties[name].c_str(), NULL);

	return f;
}

std::string Object::GetPropertyString(std::string name)
{
	return properties[name];
}


struct Layer//слои
{
	std::vector<int> tileGID;
};


class Level//главный класс - уровень
{
public:
	int tileWidth, tileHeight;
	sf::Image tileMap_image;
	sf::Texture tileMap_texture;
	sf::Sprite tileMap_sprite;
	bool LoadFromFile(std::string filename, std::string tileset_path, sf::RenderWindow* l_window);//возвращает false если не получилось загрузить
	std::vector<sf::IntRect> num_tile;
	sf::IntRect GetTileSize;//получаем размер тайла
	sf::RenderWindow* buf_window;
	void Draw();
	Level();
	void L_Free();
	Object GetObject(std::string name);
	std::vector<Layer> layers;
	std::vector<int> objlayers;
	std::vector<Object> getObjects() const { return objects; } // геттер для доступа к вектору объектов

	int width, height;//в tmx файле width height в начале,затем размер тайла
private:
	int firstTileID;//получаем айди первого тайла
	std::vector <sf::Rect <int> > solidObjects;
	//This stores all objects (including 'solid' types)
	std::vector <Object> objects;
	void LoadObjects(TiXmlElement* map); // Функция загрузки объектов
};
///////////////////////////////////////
void Level::LoadObjects(TiXmlElement* map) {
	std::cout << "Starting to load objects from XML..." << std::endl;

	for (TiXmlElement* objectGroupElement = map->FirstChildElement("objectgroup"); objectGroupElement != nullptr; objectGroupElement = objectGroupElement->NextSiblingElement("objectgroup")) {
		std::cout << "Found an object group." << std::endl;
		
		for (TiXmlElement* objectElement = objectGroupElement->FirstChildElement("object"); objectElement != nullptr; objectElement = objectElement->NextSiblingElement("object")) {
			std::cout << "Found an object: ";
			Object object;
			object.name = objectElement->Attribute("name") ? objectElement->Attribute("name") : "";
			std::cout << "Name: " << object.name << ", ";

			object.type = objectElement->Attribute("type") ? objectElement->Attribute("type") : "";
			std::cout << "Type: " << object.type << std::endl;

			object.rect.left = atoi(objectElement->Attribute("x"));
			object.rect.top = atoi(objectElement->Attribute("y"));
			object.rect.width = atoi(objectElement->Attribute("width"));
			object.rect.height = atoi(objectElement->Attribute("height"));

			std::cout << "Object rectangle: (" << object.rect.left << ", " << object.rect.top << ") - (" << object.rect.width << ", " << object.rect.height << ")" << std::endl;

			// Загрузка свойств объекта
			TiXmlElement* propertiesElement = objectElement->FirstChildElement("properties");
			if (propertiesElement) {
				std::cout << "Loading object properties." << std::endl;
				for (TiXmlElement* propertyElement = propertiesElement->FirstChildElement("property"); propertyElement != nullptr; propertyElement = propertyElement->NextSiblingElement("property")) {
					std::string propertyName = propertyElement->Attribute("name");
					std::string propertyValue = propertyElement->Attribute("value");
					object.properties[propertyName] = propertyValue;
					std::cout << "  Property: " << propertyName << " = " << propertyValue << std::endl;
				}
			}
			else {
				std::cout << "Object has no properties." << std::endl;
			}

			objects.push_back(object);
			std::cout << "Object added to the objects vector." << std::endl;
		}
	}

	std::cout << "Finished loading objects from XML. Total objects loaded: " << objects.size() << std::endl;
}

bool Level::LoadFromFile(std::string filename, std::string tileset_path, sf::RenderWindow* l_window)//двоеточия-обращение к методам класса вне класса 
{
	buf_window = l_window;
	TiXmlDocument levelFile(filename.c_str());//загружаем файл в TiXmlDocument
	// загружаем XML-карту
	if (!levelFile.LoadFile())//если не удалось загрузить карту
	{
		std::cout << "Loading level \"" << filename << "\" failed." << std::endl;//выдаем ошибку
		return false;
	}
	// работаем с контейнером map
	TiXmlElement* map;
	map = levelFile.FirstChildElement("map");
	// пример карты: <map version="1.0" orientation="orthogonal"
	// width="10" height="10" tilewidth="34" tileheight="34">
	width = atoi(map->Attribute("width"));//извлекаем из нашей карты ее свойства
	std::cout << "width level = " << width << std::endl;
	LEVEL_WIDTH = width;
	height = atoi(map->Attribute("height"));//те свойства, которые задавали при работе в 
	std::cout << "height level = " << height << std::endl;
	LEVEL_HEIGHT = height;
	tileWidth = atoi(map->Attribute("tilewidth"));//тайлмап редакторе
	std::cout << "tile Width = " << tileWidth << std::endl;
	tileHeight = atoi(map->Attribute("tileheight"));
	std::cout << "tile Height = " << tileHeight << std::endl;
	// Берем описание тайлсета и идентификатор первого тайла
	TiXmlElement* tilesetElement;
	tilesetElement = map->FirstChildElement("tileset");
	firstTileID = atoi(tilesetElement->Attribute("firstgid"));

	// source - путь до картинки в контейнере image
	//TiXmlElement* image;
	//image = tilesetElement->FirstChildElement("image");
	//std::string imagepath = image->Attribute("source");
	// пытаемся загрузить тайлсет
	if (!tileMap_image.loadFromFile(tileset_path.c_str()))
	{

		std::cout << "cant load tileMap_texture";
	}
	else
	{
		tileMap_texture.loadFromImage(tileMap_image);
		tileMap_sprite.setTexture(tileMap_texture);
	}
	
	
	// получаем количество столбцов и строк тайлсета
	int columns = tileMap_texture.getSize().x / tileWidth;
	int rows = tileMap_texture.getSize().y / tileHeight;
	std::cout << "columns = " << columns << std::endl;
	std::cout << "rows = " << rows << std::endl;

	
	//Загрузка номеров тайлов в вектор из прямоугольников изображений (TextureRect)

	GetTileSize.width = tileWidth;
	GetTileSize.height = tileHeight;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			sf::IntRect rect;
			rect.width = j * tileWidth;
			rect.height = i * tileHeight;
			num_tile.push_back(rect);

		}

	}
	// работа со слоями

	// Получить все элементы слоя ("layer")
	std::vector<TiXmlElement*> layerElements;
	for (TiXmlElement* layerElement = map->FirstChildElement("layer"); layerElement != nullptr; layerElement = layerElement->NextSiblingElement("layer"))
	{
		layerElements.push_back(layerElement);
	}
	// Для каждого слоя
	for (auto layerElement : layerElements)
	{
		Layer layer;
		
		
		// Получить данные слоя (значения тайлов)
		TiXmlElement* dataElement = layerElement->FirstChildElement("data");
		const char* data = dataElement->GetText();
		
		// Разбить данные на отдельные значения тайлов
		
		int buffer_0 = 0;
		
		std::vector<int> tileGIDs;
		std::stringstream ss(data);
		int tileGID = 0;
		char delimiter;
		while (ss >> tileGID)
		{
			ss >> delimiter; // Пропустить запятую
			tileGIDs.push_back(tileGID);
			layer.tileGID.push_back(tileGID);
		}
		
		std::cout << "Number of layer:" << layers.size() << std::endl;

		std::cout << "layer name: " << layerElement->Attribute("name") << std::endl;
		// Вывести значения тайлов
		for (int i = 0; i < layer.tileGID.size(); i++)
		{
			
			std::cout << layer.tileGID[i] << " ";
			
			if ((i + 1) % height == 0)
			{
				std::cout << std::endl;
			}
			
		}

		layers.push_back(layer);
		std::cout << std::endl;
	}

	//Objects
	LoadObjects(map);

	return true;
	


}

Level::Level()
{

};



void Level::Draw()
{
	// Добавляем небольшой запас вокруг видимой области для плавности
	viewBounds.left -= tileWidth;
	viewBounds.top -= tileHeight;
	viewBounds.width += tileWidth * 2;
	viewBounds.height += tileHeight * 2;

	// рисуем все тайлы (объекты не рисуем!)
	for (int l = 0; l < layers.size(); l++)
	{
		int h = 0;
		int w = 0;
		for (int i = 0; i < (layers[l].tileGID.size()); i++)
		{

			if (layers[l].tileGID[i] != 0)
			{
				GetTileSize = num_tile[layers[l].tileGID[i] - 1];
				tileMap_sprite.setTextureRect(sf::IntRect(GetTileSize.width, GetTileSize.height, tileWidth, tileHeight));
				
				// Вычисляем мировые координаты тайла
				float tileWorldX = w * tileWidth;
				float tileWorldY = h * tileHeight;

				 

				// Проверяем пересечение с видимой областью
				if (viewBounds.intersects(tileMap_sprite.getGlobalBounds()))
				{
					GetTileSize = num_tile[layers[l].tileGID[i] - 1];
					tileMap_sprite.setTextureRect(sf::IntRect(
						GetTileSize.width,
						GetTileSize.height,
						tileWidth,
						tileHeight
					));

					tileMap_sprite.setPosition(tileWorldX, tileWorldY);
					buf_window->draw(tileMap_sprite);
				}
				
					
				

			};
			w++;
			if (w == width)
			{
				w = 0;
				h++;
			}
		}
	};
	
}

/*
tileMap_texture.SetPosition((w * tileWidth) * Camera_scale + SCREEN_WIDTH / 2, (h * tileHeight) * Camera_scale + SCREEN_HEIGHT / 2, camera);
				if ((camera.x - tileMap_texture.GetPosition().x) < camera.x - 64 && (camera.x + tileMap_texture.GetPosition().x) < camera.x + SCREEN_WIDTH - 64 &&
					(camera.y - tileMap_texture.GetPosition().y) < camera.y - 64 && (camera.y + tileMap_texture.GetPosition().y) < camera.y + SCREEN_HEIGHT - 64)
				{
					tileMap_texture.t_Draw();
				}

*/

void Level::L_Free()
{
	// удаление массивов    
	


}
#endif