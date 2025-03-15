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

//Entity player;

struct Object
{
	int GetPropertyInt(std::string name);//номер свойства объекта в нашем списке
	float GetPropertyFloat(std::string name);
	std::string GetPropertyString(std::string name);
	std::string name;//объявили переменную name типа string
	std::string type;//а здесь переменную type типа string
	std::map<std::string, std::string> properties;//создаём ассоциатиный массив. ключ - строковый тип, значение - строковый
};
struct Layer//слои
{
	int opacity;//непрозрачность слоя
	std::vector<int> tileGID;
};


class Level//главный класс - уровень
{
public:
	sf::Image tileMap_image;
	sf::Texture tileMap_texture;
	sf::Sprite tileMap_sprite;
	bool LoadFromFile(std::string filename, std::string tileset_path, sf::RenderWindow* l_window);//возвращает false если не получилось загрузить
	Object GetObject(std::string name);
	std::vector<Object> GetObjects(std::string name);//выдаем объект в наш уровень
	std::vector<Object> GetAllObjects();//выдаем все объекты в наш уровень
	std::vector<sf::IntRect> num_tile;
	sf::IntRect GetTileSize;//получаем размер тайла
	sf::RenderWindow* buf_window;
	void Draw();
	Level();
	void L_Free();
private:
	int width, height, tileWidth, tileHeight;//в tmx файле width height в начале,затем размер тайла
	int firstTileID;//получаем айди первого тайла
	std::vector<Object> objects;//массив типа Объекты, который мы создали
	std::vector<Layer> layers;
};
///////////////////////////////////////
int Object::GetPropertyInt(std::string name)//возвращаем номер свойства в нашем списке
{
	return atoi(properties[name].c_str());
}
float Object::GetPropertyFloat(std::string name)
{
	return strtod(properties[name].c_str(), NULL);
}
std::string Object::GetPropertyString(std::string name)//получить имя в виде строки.вроде понятно
{
	return properties[name];
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
	std::cout << "num_tile_0_w = " << num_tile[0].width << std::endl;
	std::cout << "num_tile_0_h = " << num_tile[0].height << std::endl;


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
			if (layer.tileGID[i] != 0)
			{
			std::cout << layer.tileGID[i] << " ";
			}
			if ((i + 1) % height == 0)
			{
				std::cout << std::endl;
			}
			
		}

		layers.push_back(layer);
		std::cout << std::endl;
	}


}

/*
Object Level::GetObject(std::string name)
{
	// только первый объект с заданным именем
	for (int i = 0; i < objects.size(); i++)
		if (objects[i].name == name)
			return objects[i];
}
std::vector<Object> Level::GetObjects(std::string name)
{
	// все объекты с заданным именем
	std::vector<Object> vec;
	for (int i = 0; i < objects.size(); i++)
		if (objects[i].name == name)
			vec.push_back(objects[i]);
	return vec;
}
std::vector<Object> Level::GetAllObjects()
{
	return objects;
};
*/
Level::Level()
{

};


void Level::Draw()
{
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

				 

				// Проверяем, входит ли тайл в область видимости камеры
				
				 

					// Смещаем позицию тайла
					float tileScreenX = tileWorldX;
					float tileScreenY = tileWorldY; 

					tileMap_sprite.setPosition(tileScreenX, tileScreenY);
					buf_window->draw(tileMap_sprite);
				

			};
			w++;
			if (w == width)
			{
				w = 0;
				h++;
			}
		}
	};

	/// Рисование уровня
	/*
	tileMap_texture.SetPosition((w * tileWidth) + SCREEN_WIDTH / 2, (h * tileHeight) * Camera_scale + SCREEN_HEIGHT / 2, camera);
	GetTileSize = num_tile[layers[0].tileGID[71] - 1];

	tileMap_texture.Set_TextureRect(GetTileSize);

	tileMap_texture.SetPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, camera);
	tileMap_texture.t_Draw();
	*/
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