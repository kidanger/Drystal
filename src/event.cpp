#include <SDL/SDL.h>

#include "event.hpp"
#include "engine.hpp"

EventManager::EventManager(Engine& eng) : engine(eng)
{
}

void EventManager::handle_event(const SDL_Event& event)
{
	switch(event.type)
	{
		case SDL_QUIT:
			engine.stop();
			break;
		case SDL_KEYUP:
			break;
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					engine.stop();
					break;
				default:
					break;
			}
			break;
		case SDL_MOUSEMOTION:
			engine.mouse_motion(event.button.x, event.button.y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			engine.mouse_press(event.button.x, event.button.y, event.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			break;
		case SDL_VIDEORESIZE:
			engine.event_resize(event.resize.w, event.resize.h);
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

