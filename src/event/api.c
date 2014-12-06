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
#include "event_bind.h"
#include "event.h"
#include "api.h"

BEGIN_MODULE(event)
	DECLARE_FUNCTION(set_relative_mode)
	DECLARE_CONSTANT(BUTTON_LEFT)
	DECLARE_CONSTANT(BUTTON_MIDDLE)
	DECLARE_CONSTANT(BUTTON_RIGHT)
	DECLARE_CONSTANT(WHEEL_UP)
	DECLARE_CONSTANT(WHEEL_DOWN)
END_MODULE()
