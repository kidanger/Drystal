#ifndef EVENT_HPP
#define EVENT_HPP

union SDL_Event;

class Engine;

class EventManager
{
	private:
		Engine& engine;

		void handle_event(const SDL_Event& event);

	public:
		EventManager(Engine&);

		void pull();

	friend class Engine;
};

#endif
