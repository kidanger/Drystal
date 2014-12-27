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
#include "module.h"
#include "system_bind.h"
#include "api.h"

BEGIN_MODULE(particle)
	DECLARE_FUNCTION(new_system)

	BEGIN_CLASS(system)
		ADD_METHOD(system, emit)
		ADD_METHOD(system, start)
		ADD_METHOD(system, stop)
		ADD_METHOD(system, reset)

		ADD_METHOD(system, draw)
		ADD_METHOD(system, update)
		ADD_METHOD(system, add_size)
		ADD_METHOD(system, add_color)
		ADD_METHOD(system, clear_sizes)
		ADD_METHOD(system, clear_colors)
		ADD_METHOD(system, set_texture)

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

