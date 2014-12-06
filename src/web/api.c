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
#include <stdbool.h>

#include "module.h"
#include "web_bind.h"
#include "api.h"

BEGIN_MODULE(web)
#ifdef EMSCRIPTEN
	DECLARE_BOOLEAN(is_web, true)
#else
	DECLARE_BOOLEAN(is_web, false)
#endif
	DECLARE_FUNCTION(wget)
	DECLARE_FUNCTION(run_js)
END_MODULE()

