#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "ressource.hpp"

SDL_Surface* get_image(const char* filename)
{
	SDL_Surface *surf = IMG_Load(filename);

	if (surf != nullptr)
	{
		return surf;
	}
	return nullptr;
}

