#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "position.hpp"

class Sprite;

class Entity
{
	private:
		const Sprite* sprite;
		Position position;

	public:
		Position& get_position() { return position; }

		void set_sprite(const Sprite*);
		const Sprite& get_sprite() const { return *sprite; };
};

#endif
