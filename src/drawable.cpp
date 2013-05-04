#include <string>
#include <iostream>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "display.hpp"
#include "drawable.hpp"
#include "ressource.hpp"


Sprite::Sprite(const std::string& filename)
{
	this->surface = get_image(filename);
}

void Sprite::draw(Display& display, const Position& pos) const
{
	display.blit(this->surface, pos, bounds);
}
