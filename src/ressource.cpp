#include <string>
#include <iostream>
#include <map>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "emscripten.h"
#include "ressource.hpp"
#include "drawable.hpp"

std::map<std::string, SDL_Surface*> image_cache;

SDL_Surface* get_image(const std::string& filename)
{
	std::string fullname = "data/" + filename;

	if (image_cache.count(fullname) > 0)
	{
		return image_cache[fullname];
	}

	SDL_Surface *surf, *optimized = nullptr;
	surf = IMG_Load(fullname.c_str());

	if (surf != nullptr)
	{
		optimized = surf;//SDL_DisplayFormat(surf);

		//SDL_FreeSurface(surf);
		image_cache[fullname] = optimized;
		std::cout << "Sprite: " << fullname << " loaded" << std::endl;
		return optimized;
	}
	else
	{
		std::cerr << "Sprite: " << fullname << " " << SDL_GetError() << std::endl;
	}
	return nullptr;
}

const Sprite* get_sprite(enum SpriteEnum spe)
{
	Sprite* spr = new Sprite("image.png");
	switch (spe)
	{
		case HERO:
			spr->set_bounds(0, 0, 32, 32);
	}
	return spr;
}

