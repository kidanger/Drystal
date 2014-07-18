#include "module.hpp"

BEGIN_MODULE(particle)
	DECLARE_FUNCTION(new_system)

	BEGIN_CLASS(system)
		ADD_METHOD(system, start)
		ADD_METHOD(system, pause)
		ADD_METHOD(system, emit)
		ADD_METHOD(system, stop)

		ADD_METHOD(system, draw)
		ADD_METHOD(system, update)
		ADD_METHOD(system, is_running)
		ADD_METHOD(system, set_running)
		ADD_METHOD(system, add_size)
		ADD_METHOD(system, add_color)

		ADD_GETSET(system, position)
		ADD_GETSET(system, offset)
		ADD_GETSET(system, emission_rate)

#define ADD_MINMAX(name) \
		ADD_GETSET(system, min_##name) \
		ADD_GETSET(system, max_##name)
		ADD_MINMAX(lifetime)
		ADD_MINMAX(direction)
		ADD_MINMAX(initial_acceleration)
		ADD_MINMAX(initial_velocity)
#undef ADD_GETSET

		ADD_GC(free_system)
	REGISTER_CLASS(system, "System")
END_MODULE()

