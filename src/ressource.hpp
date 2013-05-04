#ifndef RESSOURCE_HPP
#define RESSOURCE_HPP

#include <string>

struct SDL_Surface;

#include "drawable.hpp"

SDL_Surface* get_image(const std::string& filename);
const Sprite* get_sprite(enum SpriteEnum spe);


#endif
