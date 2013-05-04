#ifndef EVENT_HPP
#define EVENT_HPP

#include <vector>

union SDL_Event;

class Engine;

class EventManager;
typedef void(*mouse_move_callback)(void*, EventManager&, int, int);
typedef void(*mouse_click_callback)(void*, EventManager&, int, int);

enum EventType
{
	MOUSE_MOVE, MOUSE_CLICK
};

class EventManager
{
	private:
		Engine& engine;
		bool keys[256];
		bool up, down, right, left;
		short mouse_x, mouse_y;

		std::vector<std::pair<mouse_move_callback, void*>> mouse_move;
		std::vector<std::pair<mouse_click_callback, void*>> mouse_click;

		void handle_event(const SDL_Event& event);

	public:
		EventManager(Engine&);
		//~EventManager();

		void pull();
		void add_mouse_move_callback(mouse_move_callback, void *data);
		void add_mouse_click_callback(mouse_click_callback, void *data);
		void remove_mouse_click_callback(mouse_click_callback);


	friend class Engine;
};

#endif
