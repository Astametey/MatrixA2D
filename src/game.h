#include "view.h"
#include "level.h"

//________GLOBAL VALUE________
Level test_level;
// Создание окна для рисования

void resurce_init(sf::RenderWindow* l_win)
{
	test_level.LoadFromFile("resources//levels//test_map.tmx", "resources//levels//tile set.png", l_win);
}

void game_step()
{

};

void game_draw()
{
	test_level.Draw();
}