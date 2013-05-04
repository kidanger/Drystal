#include "ressource.hpp"
#include "display.hpp"
#include "drawable.hpp"
#include "engine.hpp"
#include "event.hpp"
#include "position.hpp"
#include "entity.hpp"

#include "game/gameimpl.hpp"


void Game::set_hero_position(EventManager& event, int mousex, int mousey)
{
	(void)event;
	Position& pos = hero->get_position();
	pos.x = mousex;
	pos.y = mousey;
}

void Game::setup(Engine&)
{
}

void Game::preload()
{
	get_image("image.png");
	const Sprite* sp = get_sprite(HERO);
	hero = new Entity();
	hero->set_sprite(sp);
}

void Game::update(Engine& engine)
{
	(void) engine;
	Position& pos = hero->get_position();
	pos.y += 10;
}

void Game::draw(Display& display)
{
	display.draw(hero->get_sprite(), hero->get_position());
}

void Game::clean(Engine&)
{
}

