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
#include "module.hpp"
#include "font_bind.hpp"

BEGIN_MODULE(truetype)
	DECLARE_FUNCTION(load_font)

	BEGIN_CLASS(font)
		ADD_METHOD(font, draw)
		ADD_METHOD(font, draw_plain)
		ADD_METHOD(font, sizeof)
		ADD_METHOD(font, sizeof_plain)
		ADD_GC(free_font)
	REGISTER_CLASS(font, "Font")
END_MODULE()

