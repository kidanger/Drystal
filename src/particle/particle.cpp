/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cmath>
#include <cassert>
#include <cstdlib> // random

#include <lua.hpp>

#include "engine.hpp"
#include "api.hpp"

#define RAND(a, b) (((float) rand()/RAND_MAX) * ((b) - (a)) + (a))

class System;
class Particle {
public:
	bool dead;

	float x, y;
	float vel;
	float accel;
	float dir_angle;

	float life, lifetime;

	int size_state;
	float sizeseed;

	int color_state;
	float rseed;
	float gseed;
	float bseed;

	void update(System& sys, float dt);
};

#define MAX_SIZES 5
#define MAX_COLORS 5

struct Size {
	float at;
	float min, max;
};

struct Color {
	float at;
	float min_r, max_r;
	float min_g, max_g;
	float min_b, max_b;
};

class System {
private:
	System(const System&);
	System& operator=(const System&);

	Particle* particles;
public:

	int cur_size;
	Size sizes[MAX_SIZES];

	int cur_color;
	Color colors[MAX_COLORS];

	bool running;

	int size;
	int used;

	int x, y;
	int offx, offy;

	float min_direction, max_direction;
	float min_lifetime, max_lifetime;

	float min_initial_acceleration, max_initial_acceleration;
	float min_initial_velocity, max_initial_velocity;

	float emission_rate;
	float emit_counter;

	System(int x, int y) :
		particles(NULL),
		cur_size(0), cur_color(0),
		running(false),
		size(256), used(0),
		x(x), y(y), offx(0), offy(0),
		min_direction(0), max_direction(0),
		min_lifetime(0), max_lifetime(0),
		min_initial_acceleration(0), max_initial_acceleration(0),
		min_initial_velocity(0), max_initial_velocity(0),
		emission_rate(0), emit_counter(0)
	{
		particles = new Particle[size];
		for (int i = 0; i < size; i++)
			particles[i].dead = true;
        }

	~System()
	{
		delete[] particles;
	}

	void start()
	{
		running = true;
	}
	void pause()
	{
		running = false;
	}
	void stop()
	{
		for (int i = 0; i < used; i++) {
			Particle* p = &particles[i];
			p->dead = true;
		}
		used = 0;
		running = false;
	}

	void draw(float dx, float dy)
	{
		Engine& engine = get_engine();

		for (int i = used-1; i >= 0; i--) {
			Particle* p = &particles[i];

			float liferatio = 1 - p->life / p->lifetime;

			float _size;
			{
				Size& sA = sizes[p->size_state];
				Size& sB = sizes[p->size_state + 1];

				float ratio = (liferatio - sA.at) / (sB.at - sA.at);

				float sizeA = p->sizeseed * (sA.max - sA.min) + sA.min;
				float sizeB = p->sizeseed * (sB.max - sB.min) + sB.min;
				_size = sizeA * (1 - ratio) + sizeB * ratio;
			}

			float r, g, b;
			{
				Color& cA = colors[p->color_state];
				Color& cB = colors[p->color_state + 1];

				float ratio = (liferatio - cA.at) / (cB.at - cA.at);

				float colrA = p->rseed * (cA.max_r - cA.min_r) + cA.min_r;
				float colrB = p->rseed * (cB.max_r - cB.min_r) + cB.min_r;
				r = colrA * (1 - ratio) + colrB * ratio;

				float colgA = p->gseed * (cA.max_g - cA.min_g) + cA.min_g;
				float colgB = p->gseed * (cB.max_g - cB.min_g) + cB.min_g;
				g = colgA * (1 - ratio) + colgB * ratio;

				float colbA = p->bseed * (cA.max_b - cA.min_b) + cA.min_b;
				float colbB = p->bseed * (cB.max_b - cB.min_b) + cB.min_b;
				b = colbA * (1 - ratio) + colbB * ratio;
			}


			engine.display.set_color(r, g, b);
			engine.display.set_point_size(_size);
			engine.display.draw_point(dx + p->x, dy + p->y);
		}
	}

	void emit()
	{
		if (used == size) {
			return;
		}
		Particle* p = &particles[used];
		p->x = x + RAND(-offx, offx);
		p->y = y + RAND(-offy, offy);
		p->sizeseed = (float) rand() / RAND_MAX;
		p->rseed = (float) rand() / RAND_MAX;
		p->gseed = (float) rand() / RAND_MAX;
		p->bseed = (float) rand() / RAND_MAX;
		p->color_state = 0;
		p->size_state = 0;

		p->dir_angle = RAND(min_direction, max_direction);
		p->accel = RAND(min_initial_acceleration, max_initial_acceleration);
		p->vel = RAND(min_initial_velocity, max_initial_velocity);

		p->lifetime = RAND(min_lifetime, max_lifetime);
		p->life = p->lifetime;

		p->dead = false;

		used += 1;
	}

	void update(float dt)
	{
		for (int i = 0; i < used; i++) {
			Particle* p = &particles[i];
			p->update(*this, dt);
		}

		for (int i = 0; i < used; i++) {
			Particle* p = &particles[i];
			if (p->life <= 0) {
				p->dead = true;
				particles[i] = particles[used - 1];
				used -= 1;
				i -= 1;
			}
		}

		if (running) {
			float rate = 1.0f / emission_rate;
			emit_counter += dt;
			if (emit_counter > rate and used < size) {
				emit();
				emit_counter -= rate;
			}
		}
	}

	void add_size(float at, float min, float max)
	{
		int i = 0;
	//	while (i < cur_size and sizes[i].at < at) {
	//		i += 1;
	//	}
		i = cur_size;
		assert(i != MAX_SIZES);
		if (i == MAX_SIZES)
			return;
		sizes[i].at = at;
		sizes[i].min = min;
		sizes[i].max = max;
		cur_size += 1;
	}

	void add_color(float at, float min_r, float max_r, float min_g, float max_g, float min_b, float max_b)
	{
		int i = cur_color;
		assert(i != MAX_COLORS);
		if (i == MAX_COLORS)
			return;
		colors[i].at = at;
		colors[i].min_r = min_r;
		colors[i].max_r = max_r;
		colors[i].min_g = min_g;
		colors[i].max_g = max_g;
		colors[i].min_b = min_b;
		colors[i].max_b = max_b;
		cur_color += 1;
	}

};
void Particle::update(System& sys, float dt)
{
	life -= dt;

	vel += accel * dt;
	x += vel * cos(dir_angle) * dt;
	y += vel * sin(dir_angle) * dt;

	float liferatio = 1 - life / lifetime;
	if (liferatio > sys.sizes[size_state + 1].at and size_state < sys.cur_size) {
		size_state += 1;
	}
	if (liferatio > sys.colors[color_state + 1].at and color_state < sys.cur_color) {
		color_state += 1;
	}
}

DECLARE_PUSHPOP(System, system)

int mlua_new_system(lua_State* L)
{
        int x;
        int y;

	x = luaL_checknumber(L, 1);
	y = luaL_checknumber(L, 2);

	System* system = new System(x, y);

	system->min_direction = 0;
	system->max_direction = M_PI * 2;

	system->sizes[0].at = 0;
	system->sizes[0].min = 5;
	system->sizes[0].max = 5;
	system->sizes[1].at = 1;
	system->sizes[1].min = 5;
	system->sizes[1].max = 5;
	system->min_lifetime = 3;
	system->max_lifetime = 10;

	system->min_initial_acceleration = RAND(-10, 10);
	system->max_initial_acceleration = system->min_initial_acceleration + 3;
	system->min_initial_velocity = RAND(10, 100);
	system->max_initial_velocity = system->min_initial_velocity + RAND(10, 100);

	system->colors[0].at = 0;
	system->colors[0].min_r = RAND(0, 125);
	system->colors[0].max_r = system->colors[0].min_r + RAND(0, 50);
	system->colors[0].min_g = RAND(0, 125);
	system->colors[0].max_g = system->colors[0].min_g + RAND(0, 50);
	system->colors[0].min_b = RAND(0, 125);
	system->colors[0].max_b = system->colors[0].min_b + RAND(0, 50);
	system->colors[1].at = 1;
	system->colors[1].min_r = RAND(0, 125);
	system->colors[1].max_r = system->colors[0].min_r + RAND(0, 50);
	system->colors[1].min_g = RAND(0, 125);
	system->colors[1].max_g = system->colors[0].min_g + RAND(0, 50);
	system->colors[1].min_b = RAND(0, 125);
	system->colors[1].max_b = system->colors[0].min_b + RAND(0, 50);

	system->emission_rate = RAND(1, 19);
	system->offx = 0;
	system->offy = 0;

	push_system(L, system);
	return 1;
}

int mlua_set_position_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	system->x = x;
	system->y = y;
	return 0;
}
int mlua_get_position_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	lua_pushnumber(L, system->x);
	lua_pushnumber(L, system->y);
	return 2;
}
int mlua_set_offset_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	lua_Number ox = luaL_checknumber(L, 2);
	lua_Number oy = luaL_checknumber(L, 3);
	system->offx = ox;
	system->offy = oy;
	return 0;
}
int mlua_get_offset_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	lua_pushnumber(L, system->offx);
	lua_pushnumber(L, system->offy);
	return 2;
}

#define GETSET(attr) \
	int mlua_get_##attr##_system(lua_State* L) \
	{ \
	    System* system = pop_system(L, 1);\
		lua_pushnumber(L, system->attr); \
		return 1; \
	} \
	int mlua_set_##attr##_system(lua_State* L) \
	{ \
	    System* system = pop_system(L, 1);\
		lua_Number attr = luaL_checknumber(L, 2); \
		system->attr = attr; \
		return 0; \
	}

GETSET(min_lifetime)
GETSET(max_lifetime)
GETSET(min_direction)
GETSET(max_direction)
GETSET(min_initial_acceleration)
GETSET(max_initial_acceleration)
GETSET(min_initial_velocity)
GETSET(max_initial_velocity)
GETSET(emission_rate)


int mlua_update_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	lua_Number dt = luaL_checknumber(L, 2);
	system->update(dt);
	return 0;
}

#define ACTION(action) \
	int mlua_##action##_system(lua_State* L) \
	{ \
	    System* system = pop_system(L, 1);\
		system->action(); \
		return 0; \
	}

ACTION(emit)
ACTION(start)
ACTION(pause)
ACTION(stop)

int mlua_draw_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	lua_Number dx = 0;
	lua_Number dy = 0;
	if (lua_gettop(L) > 1) {
		dx = luaL_checknumber(L, 2);
		dy = luaL_checknumber(L, 3);
	}
	system->draw(dx, dy);
	return 0;
}

int mlua_is_running_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	bool running = system->running;
	lua_pushboolean(L, running);
	return 1;
}
int mlua_set_running_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	bool running = lua_toboolean(L, 2);
	system->running = running;
	return 0;
}

int mlua_add_size_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	lua_Number at_lifetime = luaL_checknumber(L, 2);
	lua_Number min = luaL_checknumber(L, 3);
	lua_Number max = min;
	if (lua_gettop(L) > 3) {
		max = luaL_checknumber(L, 4);
	}
	system->add_size(at_lifetime, min, max);
	return 0;
}

int mlua_add_color_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	lua_Number at_lifetime = luaL_checknumber(L, 2);
	if (lua_gettop(L) == 5) {
		lua_Number r = luaL_checknumber(L, 3);
		lua_Number g = luaL_checknumber(L, 4);
		lua_Number b = luaL_checknumber(L, 5);
		system->add_color(at_lifetime, r, r, g, g, b, b);
	} else {
		lua_Number minr = luaL_checknumber(L, 3);
		lua_Number maxr = luaL_checknumber(L, 4);
		lua_Number ming = luaL_checknumber(L, 5);
		lua_Number maxg = luaL_checknumber(L, 6);
		lua_Number minb = luaL_checknumber(L, 7);
		lua_Number maxb = luaL_checknumber(L, 8);
		system->add_color(at_lifetime, minr, maxr, ming, maxg, minb, maxb);
	}
	return 0;
}

int mlua_free_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	delete system;
	return 0;
}

