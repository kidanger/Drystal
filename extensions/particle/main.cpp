#include <cmath>
#include <cassert>
#include <cstdlib> // random

#include <lua.hpp>

#include "engine.hpp"

#define DECLARE_FUNCTION(x) {#x, particle_##x}
#define DECLARE_GETSET(x) DECLARE_FUNCTION(get_##x), DECLARE_FUNCTION(set_##x)

#define RAND(a, b) (((float) rand()/RAND_MAX) * ((b) - (a)) + (a))

class System;
class Particle {
public:
	bool dead = true;

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
	Particle* particles;
public:

	int cur_size = 0;
	Size sizes[MAX_SIZES];

	int cur_color = 0;
	Color colors[MAX_COLORS];

	bool running = false;

	int size = 256;
	int used = 0;

	int x, y;
	int offx, offy;

	float min_direction, max_direction;
	float min_lifetime, max_lifetime;

	float min_initial_acceleration, max_initial_acceleration;
	float min_initial_velocity, max_initial_velocity;

	float emission_rate;
	float emit_counter = 0;

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

	void allocate()
	{
		particles = new Particle[size];
		used = 0;
	}

	void draw(float dx, float dy)
	{
		Engine& engine = get_engine();

		for (int i = used-1; i >= 0; i--) {
			Particle* p = &particles[i];

			float liferatio = 1 - p->life / p->lifetime;

			float size;
			{
				Size& sA = sizes[p->size_state];
				Size& sB = sizes[p->size_state + 1];

				float ratio = (liferatio - sA.at) / (sB.at - sA.at);

				float sizeA = p->sizeseed * (sA.max - sA.min) + sA.min;
				float sizeB = p->sizeseed * (sB.max - sB.min) + sB.min;
				size = sizeA * (1 - ratio) + sizeB * ratio;
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
			engine.display.set_point_size(size);
			engine.display.draw_point(dx + p->x, dy + p->y);
		}
	}

	void emit()
	{
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

int particle_new_system(lua_State* L)
{
	System* system = new System;

	system->x = luaL_checknumber(L, 1);
	system->y = luaL_checknumber(L, 2);

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

	system->allocate();
	lua_pushlightuserdata(L, system);
	return 1;
}

int particle_set_position(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	system->x = x;
	system->y = y;
	return 0;
}
int particle_get_position(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	lua_pushnumber(L, system->x);
	lua_pushnumber(L, system->y);
	return 2;
}
int particle_set_offset(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	lua_Number ox = luaL_checknumber(L, 2);
	lua_Number oy = luaL_checknumber(L, 3);
	system->offx = ox;
	system->offy = oy;
	return 0;
}
int particle_get_offset(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	lua_pushnumber(L, system->offx);
	lua_pushnumber(L, system->offy);
	return 2;
}

#define GETSET(attr) \
	int particle_get_##attr(lua_State* L) \
	{ \
		System* system = (System*) lua_touserdata(L, 1); \
		lua_pushnumber(L, system->attr); \
		return 1; \
	} \
	int particle_set_##attr(lua_State* L) \
	{ \
		System* system = (System*) lua_touserdata(L, 1); \
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


int particle_update(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	lua_Number dt = luaL_checknumber(L, 2);
	system->update(dt);
	return 0;
}

#define ACTION(action) \
	int particle_##action(lua_State* L) \
	{ \
		System* system = (System*) lua_touserdata(L, 1); \
		system->action(); \
		return 0; \
	}

ACTION(emit)
ACTION(start)
ACTION(pause)
ACTION(stop)

int particle_draw(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	lua_Number dx = 0;
	lua_Number dy = 0;
	if (lua_gettop(L) > 1) {
		dx = luaL_checknumber(L, 2);
		dy = luaL_checknumber(L, 3);
	}
	system->draw(dx, dy);
	return 0;
}

int particle_is_running(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	bool running = system->running;
	lua_pushboolean(L, running);
	return 1;
}
int particle_set_running(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	bool running = lua_toboolean(L, 2);
	system->running = running;
	return 0;
}

int particle_add_size(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	lua_Number at_lifetime = luaL_checknumber(L, 2);
	lua_Number min = luaL_checknumber(L, 3);
	lua_Number max = min;
	if (lua_gettop(L) > 3) {
		max = luaL_checknumber(L, 4);
	}
	system->add_size(at_lifetime, min, max);
	return 0;
}

int particle_add_color(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
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

//lua_Number g = luaL_checknumber(L, 4);
//lua_Number b = luaL_checknumber(L, 5);

int particle_free(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	delete system;
	return 0;
}

static const luaL_Reg lib[] =
{
	DECLARE_FUNCTION(new_system),
	DECLARE_FUNCTION(update),
	DECLARE_FUNCTION(emit),
	DECLARE_FUNCTION(start),
	DECLARE_FUNCTION(stop),
	DECLARE_FUNCTION(pause),
	DECLARE_FUNCTION(draw),
	DECLARE_FUNCTION(free),

	DECLARE_FUNCTION(is_running),
	DECLARE_FUNCTION(set_running),
	DECLARE_FUNCTION(add_size),
	DECLARE_FUNCTION(add_color),

	// yukk
	DECLARE_FUNCTION(get_position),
	DECLARE_FUNCTION(set_position),
	DECLARE_FUNCTION(get_offset),
	DECLARE_FUNCTION(set_offset),
	DECLARE_GETSET(min_lifetime),
	DECLARE_GETSET(max_lifetime),
	DECLARE_GETSET(min_direction),
	DECLARE_GETSET(max_direction),
	DECLARE_GETSET(min_initial_acceleration),
	DECLARE_GETSET(max_initial_acceleration),
	DECLARE_GETSET(min_initial_velocity),
	DECLARE_GETSET(max_initial_velocity),
	DECLARE_GETSET(emission_rate),

	{NULL, NULL}
};

DEFINE_EXTENSION(particle)
{
	luaL_newlib(L, lib);
	luaL_setfuncs(L, lib, 0);

	return 1;
}


