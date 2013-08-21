#define LUA_API extern
#define DRYSTAL_EXTENSION 1

#include <lua.hpp>
#include <cmath>
#include <cstdlib>

#include "engine.hpp"

#define DECLARE_FUNCTION(x) {#x, particle_##x}

#define RAND(a, b) (((float) rand()/RAND_MAX) * ((b) - (a)) + (a))

struct Particle {
	bool dead = true;

	float size;
	float x, y;
	float vel;
	float accel;
	float dir_angle;

	float r, g, b, a;

	float life, lifetime;

	void update(float dt)
	{
		vel += accel * dt;
		x += vel * cos(dir_angle) * dt;
		y += vel * sin(dir_angle) * dt;

		life -= dt;
	}
};

struct System {
	Particle* particles;
	//Buffer* buffer;

	int size = 256;
	int used = 0;

	int x, y;
	float min_vel;
	float max_vel;

	float min_direction, max_direction;
	float min_lifetime, max_lifetime;

	float min_initial_acceleration, max_initial_acceleration;
	float min_initial_velocity, max_initial_velocity;

	float last_emit = 0;
	float time = 0;

	float min_size, max_size;

	int min_r, max_r;
	int min_g, max_g;
	int min_b, max_b;

	float emission_rate;
	float emit_counter = 0;

	~System()
	{
		delete[] particles;
	}

	void start()
	{
	}
	void pause()
	{
	}
	void resume()
	{
	}
	void stop()
	{
	}

	void allocate()
	{
		particles = new Particle[size];
		//Engine& engine = get_engine();
		//buffer = engine.display.new_buffer();
		//buffer->assert_type(POINT_BUFFER);
		used = 0;
	}

	void draw()
	{
		// TODO: handle scrolling
		Engine& engine = get_engine();
		//engine.display.draw_buffer(buffer, 0, 0);

		for (int i = 0; i < size; i++) {
			Particle* p = &particles[i];
			if (not p->dead) {
				engine.display.set_color(p->r, p->g, p->b);
				engine.display.set_alpha(p->a);
				engine.display.set_point_size(p->size);
				engine.display.draw_point(p->x, p->y);
			}
		}
	}

	void emit(int index)
	{
		Particle* p = &particles[index];
		p->x = x;
		p->y = y;
		p->size = RAND(min_size, max_size);

		p->r = RAND(min_r, max_r);
		p->g = RAND(min_g, max_g);
		p->b = RAND(min_b, max_b);
		p->a = 255;

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
		time += dt;

		int free_index = -1;
		for (int i = 0; i < size; i++) {
			Particle* p = &particles[i];
			if (not p->dead) {
				p->update(dt);

				if (p->life <= 0) {
					p->dead = true;
				}
			} else {
				free_index = i;
			}
		}

		float rate = 1.0f / emission_rate;
		emit_counter += dt;
		if (emit_counter > rate and free_index != -1) {
			emit(free_index);
			emit_counter -= rate;
		}
	}
};

int particle_new_system(lua_State* L)
{
	System* system = new System;

	system->x = luaL_checknumber(L, 1);
	system->y = luaL_checknumber(L, 2);

	system->min_direction = - M_PI / 2 - M_PI/12;
	system->max_direction = - M_PI / 2 + M_PI/12;

	system->min_size = RAND(1, 4);
	system->max_size = RAND(5, 10);
	system->min_lifetime = 3;
	system->max_lifetime = 10;

	system->min_initial_acceleration = RAND(-10, 10);
	system->max_initial_acceleration = system->min_initial_acceleration + 3;
	system->min_initial_velocity = RAND(10, 100);
	system->max_initial_velocity = system->min_initial_velocity + RAND(10, 100);

	system->min_r = RAND(0, 125);
	system->max_r = system->min_r + RAND(0, 50);
	system->min_g = RAND(0, 125);
	system->max_g = system->min_g + RAND(0, 50);
	system->min_b = RAND(0, 125);
	system->max_b = system->min_b + RAND(0, 50);

	system->emission_rate = RAND(1, 19);

	system->allocate();
	lua_pushlightuserdata(L, system);
	return 1;
}

int particle_update(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	lua_Number dt = luaL_checknumber(L, 2);
	system->update(dt);
	return 0;
}

int particle_draw(lua_State* L)
{
	System* system = (System*) lua_touserdata(L, 1);
	system->draw();
	return 0;
}

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
	DECLARE_FUNCTION(draw),
	DECLARE_FUNCTION(free),
	{NULL, NULL}
};

LUA_API "C" int luaopen_particle(lua_State *L)
{
	luaL_newlib(L, lib);
	luaL_setfuncs(L, lib, 0);

	return 1;
}


