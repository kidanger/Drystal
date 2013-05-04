#include <SDL/SDL.h>
#include <cstring> // memset

#include "event.hpp"
#include "engine.hpp"

EventManager::EventManager(Engine& eng) : engine(eng)
{
	memset(keys, false, sizeof(keys));
	mouse_x = mouse_y = 0;
	left = right = up = down = false;
}

void EventManager::handle_event(const SDL_Event& event)
{
	switch(event.type)
	{
		case SDL_QUIT:
			engine.stop();
			break;
		case SDL_KEYUP:
			switch(event.key.keysym.sym)
			{
				case SDLK_UP:
					up = false;
					break;
				case SDLK_DOWN:
					down = false;
					break;
				case SDLK_LEFT:
					left = false;
					break;
				case SDLK_RIGHT:
					right = false;
					break;
				default:
					break;
			}
			break;
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym)
			{
				case SDLK_UP:
					up = true;
					break;
				case SDLK_DOWN:
					down = true;
					break;
				case SDLK_LEFT:
					left = true;
					break;
				case SDLK_RIGHT:
					right = true;
					break;
				case SDLK_ESCAPE:
					engine.stop();
					break;
				default:
					break;
			}
			break;
		case SDL_MOUSEMOTION:
			mouse_x = event.button.x;
			mouse_y = event.button.y;
			engine.mouse_motion(mouse_x, mouse_y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			engine.mouse_press(mouse_x, mouse_y, event.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			break;
		case SDL_VIDEORESIZE:
			break;
		default:
			break;
	}
}

void EventManager::pull()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		handle_event(event);
	}
}

