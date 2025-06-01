#include "levelLoader.h"

Level::Level()
	: tileWidth(0), tileHeight(0),
	width(0), height(0),
	firstTileID(0)
{
	// Дополнительная инициализация, если нужна
}

int Object::GetPropertyInt(std::string name) const
{
	return 0;
}

float Object::GetPropertyFloat(std::string name) const {
	auto it = properties.find(name);
	if (it != properties.end()) {
		return strtod(it->second.c_str(), nullptr);
	}
	return 0.0f;
}

std::string Object::GetPropertyString(std::string name) const {
	auto it = properties.find(name);
	if (it != properties.end()) {
		return it->second;
	}
	return "";
}

void Object::setRect(float width, float height, float x, float y)
{
	rect.width = width;
	rect.height = height;
	rect.left = x;
	rect.top = y;
};

bool Level::isPositionWalkable(const sf::Vector2f& position) const
{
	// 1. Проверяем, находится ли позиция за пределами карты
	if (position.x < 0 || position.y < 0 ||
		position.x >= width * tileWidth ||
		position.y >= height * tileHeight)
	{
		return false;
	}

	// 2. Проверяем столкновение с объектами
	for (const auto& object : objects)
	{
		// Проверяем только объекты, которые являются препятствиями
		if (object.type == "solid" || object.name == "wall" || object.name == "obstacle")
		{
			if (object.rect.contains(position))
			{
				return false;
			}
		}
	}

	// 3. Проверяем тайлы слоев (например, если есть слой с непроходимыми тайлами)
	for (const auto& layer : layers)
	{
		// Вычисляем индексы тайла в сетке карты
		int tileX = static_cast<int>(position.x) / tileWidth;
		int tileY = static_cast<int>(position.y) / tileHeight;

		// Проверяем, что координаты в пределах карты
		if (tileX >= 0 && tileX < width && tileY >= 0 && tileY < height)
		{
			int tileIndex = tileY * width + tileX;

			// Проверяем GID тайла (0 обычно означает пустой тайл)
			if (tileIndex < static_cast<int>(layer.tileGID.size()) && layer.tileGID[tileIndex] != 0)
			{
				// Здесь можно добавить дополнительную проверку по свойствам тайла,
				// если у вас есть информация о непроходимых тайлах
				// Например, если определенные GID соответствуют непроходимым тайлам
				return false;
			}
		}
	}

	// Если ни одно из условий не сработало, позиция проходима
	return true;
}

bool Level::LoadFromFile(std::string filename, std::string tileset_path)//двоеточия-обращение к методам класса вне класса 
{
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
	if (!tileMap_image.loadFromFile(tileset_path.c_str())) {
		std::cerr << "Failed to load tileset: " << tileset_path << std::endl;
		return false;
	}
	tileMap_texture.loadFromImage(tileMap_image);
	tileMap_sprite.setTexture(tileMap_texture, true); // true - reset texture rect

	// получаем количество столбцов и строк тайлсета
	int columns = tileMap_texture.getSize().x / tileWidth;
	int rows = tileMap_texture.getSize().y / tileHeight;
	std::cout << "columns = " << columns << std::endl;
	std::cout << "rows = " << rows << std::endl;


	//Загрузка номеров тайлов в вектор из прямоугольников изображений (TextureRect)

	GetTileSize.width = tileWidth;
	GetTileSize.height = tileHeight;

	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < columns; x++) {
			sf::IntRect rect(
				x * tileWidth,  // left
				y * tileHeight, // top
				tileWidth,      // width
				tileHeight      // height
			);
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
		layer.name = layerElement->Attribute("name") ? layerElement->Attribute("name") : "";

		// Определяем специальные слои по имени
		layer.alwaysOnTop = (layer.name == "Overlay" || layer.name == "AbovePlayer");

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

			//std::cout << layer.tileGID[i] << " ";

			if ((i + 1) % height == 0)
			{
				//std::cout << std::endl;
			}

		}

		layers.push_back(layer);
		std::cout << std::endl;
	}

	//Objects
	LoadObjects(map);

	return true;



}

void Level::LoadObjects(TiXmlElement* map) {
	std::cout << "Starting to load objects from XML..." << std::endl;

	for (TiXmlElement* objectGroupElement = map->FirstChildElement("objectgroup"); objectGroupElement != nullptr; objectGroupElement = objectGroupElement->NextSiblingElement("objectgroup")) {
		std::cout << "Found an object group." << std::endl;

		for (TiXmlElement* objectElement = objectGroupElement->FirstChildElement("object"); objectElement != nullptr; objectElement = objectElement->NextSiblingElement("object")) {
			//std::cout << "Found an object: ";
			Object object;
			object.name = objectElement->Attribute("name") ? objectElement->Attribute("name") : "";
			//std::cout << "Name: " << object.name << ", ";

			object.type = objectElement->Attribute("type") ? objectElement->Attribute("type") : "";
			//std::cout << "Type: " << object.type << std::endl;

			object.rect.left = atoi(objectElement->Attribute("x"));
			object.rect.top = atoi(objectElement->Attribute("y"));
			object.rect.width = atoi(objectElement->Attribute("width"));
			object.rect.height = atoi(objectElement->Attribute("height"));
			object.rect.top -= tileHeight;

			//std::cout << "Object rectangle: (" << object.rect.left << ", " << object.rect.top << ") - (" << object.rect.width << ", " << object.rect.height << ")" << std::endl;

			// Загрузка свойств объекта
			TiXmlElement* propertiesElement = objectElement->FirstChildElement("properties");
			if (propertiesElement) {
				std::cout << "Loading object properties." << std::endl;
				for (TiXmlElement* propertyElement = propertiesElement->FirstChildElement("property"); propertyElement != nullptr; propertyElement = propertyElement->NextSiblingElement("property")) {
					std::string propertyName = propertyElement->Attribute("name");
					std::string propertyValue = propertyElement->Attribute("value");
					object.properties[propertyName] = propertyValue;
					//std::cout << "  Property: " << propertyName << " = " << propertyValue << std::endl;
				}
			}
			else {
				//std::cout << "Object has no properties." << std::endl;
			}
			if (object.name == "wall" || object.name == "tree") {
				object.alwaysBehind = true;
				object.rect.top += 32;
			}
			else if (object.name == "Barrel") {
				object.alwaysBehind = false; // Эти объекты могут быть и перед и за игроком
			}
			objects.push_back(object);
			//std::cout << "Object added to the objects vector." << std::endl;
		}
	}

	std::cout << "Finished loading objects from XML. Total objects loaded: " << objects.size() << std::endl;
}
std::vector<Object*> Level::GetObjectsForRendering(const sf::FloatRect& viewBounds, const sf::Vector2f& playerPos) {
	std::vector<Object*> objectsToRender;

	// Собираем все объекты, которые находятся в области видимости
	for (auto& object : objects) {
		if (viewBounds.intersects(object.rect)) {
			objectsToRender.push_back(&object);
		}
	}

	// Сортируем объекты по их нижней границе (Y + Height)
	std::sort(objectsToRender.begin(), objectsToRender.end(),
		[](const Object* a, const Object* b) {
			return (a->rect.top + a->rect.height) < (b->rect.top + b->rect.height);
		});

	return objectsToRender;
}

void Level::DrawTiles(const sf::FloatRect& viewBounds, sf::RenderWindow* window) {
	// Рисуем только тайлы уровня (самый нижний слой)
	sf::FloatRect adjustedBounds = viewBounds;
	adjustedBounds.left -= tileWidth;
	adjustedBounds.top -= tileHeight;
	adjustedBounds.width += tileWidth * 2;
	adjustedBounds.height += tileHeight * 2;

	for (int l = 0; l < layers.size(); l++) {
		int h = 0;
		int w = 0;
		for (int i = 0; i < layers[l].tileGID.size(); i++) {
			if (layers[l].tileGID[i] != 0) {
				GetTileSize = num_tile[layers[l].tileGID[i] - 1];
				tileMap_sprite.setTextureRect(sf::IntRect(
					GetTileSize.left,   // X координата тайла в тайлсете
					GetTileSize.top,    // Y координата тайла в тайлсете
					tileWidth,          // Ширина тайла
					tileHeight          // Высота тайла
				));
				tileMap_sprite.setPosition(w * tileWidth, h * tileHeight);
				if (adjustedBounds.intersects(tileMap_sprite.getGlobalBounds())) {
					window->draw(tileMap_sprite);
				}
			}
			w++; if (w == width) { w = 0; h++; }
		}
	}
}

void Level::DrawLayer(int layerIndex, const sf::FloatRect& viewBounds, sf::RenderWindow* window) {
	if (layerIndex < 0 || layerIndex >= layers.size()) return;

	Layer& layer = layers[layerIndex];
	int h = 0;
	int w = 0;

	for (int i = 0; i < layer.tileGID.size(); i++) {
		if (layer.tileGID[i] != 0) {
			GetTileSize = num_tile[layer.tileGID[i] - 1];
			tileMap_sprite.setTextureRect(sf::IntRect(
				GetTileSize.left,   // X координата тайла в тайлсете
				GetTileSize.top,    // Y координата тайла в тайлсете
				tileWidth,          // Ширина тайла
				tileHeight          // Высота тайла
			));
			tileMap_sprite.setPosition(w * tileWidth, h * tileHeight);

			if (viewBounds.intersects(tileMap_sprite.getGlobalBounds())) {
				window->draw(tileMap_sprite);
			}
		}
		w++; if (w == width) { w = 0; h++; }
	}
}
void Level::DrawLayerByName(const std::string& layerName, const sf::FloatRect& viewBounds, sf::RenderWindow* window) {
	for (int i = 0; i < layers.size(); i++) {
		if (layers[i].name == layerName) {
			DrawLayer(i, viewBounds, window);
			return;
		}
	}
}

void Level::DrawObject(const Object& object, const sf::FloatRect& viewBounds, sf::RenderWindow* window) {
	if (object.name == "Barrel") {
		// Правильное позиционирование бочки
		sf::Vector2f drawPos(object.rect.left, object.rect.top - object.rect.height + 32);
		tileMap_sprite.setTextureRect(sf::IntRect(256, 0, tileWidth, tileHeight));
		tileMap_sprite.setPosition(drawPos);

		if (viewBounds.intersects(sf::FloatRect(drawPos.x, drawPos.y,
			object.rect.width, object.rect.height))) {
			window->draw(tileMap_sprite);
		}
	}
}

void Level::DrawObjectsBehindPlayer(const sf::FloatRect& viewBounds, sf::RenderWindow* window) {
	// Рисуем объекты, которые всегда должны быть под игроком
	for (const auto& object : objects) {
		if (object.type == "solid" && object.alwaysBehind) {
			DrawObject(object, viewBounds, window);
		}
	}
}

void Level::DrawObjectsInFrontOfPlayer(const sf::FloatRect& viewBounds, const sf::Vector2f& playerPos, sf::RenderWindow* window) {
	// Рисуем объекты, которые могут быть над игроком
	for (const auto& object : objects) {
		if (object.type == "solid" && !object.alwaysBehind) {
			// Объект рисуется над игроком, если его нижняя граница выше позиции игрока
			bool shouldDrawInFront = (playerPos.y > object.rect.top + object.rect.height);
			if (shouldDrawInFront) {
				DrawObject(object, viewBounds, window);
			}
		}
	}
}

void Level::L_Free()
{
	// удаление массивов    


}
