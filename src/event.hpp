#ifndef EVENT_HPP
#define EVENT_HPP

union SDL_Event;

class Engine;

class EventManager
{
	private:
		Engine& engine;
		bool _grab_cursor;
		bool _warped;

		void handle_event(const SDL_Event& event);
		void check_grab();

	public:
		EventManager(Engine&);

		void poll();

		void grab_cursor(bool);
};

#endif
