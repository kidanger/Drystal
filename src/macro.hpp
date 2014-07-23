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

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#define BEGIN_DISABLE_WARNINGS \
	_Pragma("GCC diagnostic push")

#define DISABLE_WARNING_EFFCPP \
	_Pragma("GCC diagnostic ignored \"-Weffc++\"")
#define DISABLE_WARNING_STRICT_ALIASING \
	_Pragma("GCC diagnostic ignored \"-Wstrict-aliasing\"")

#define END_DISABLE_WARNINGS \
	_Pragma("GCC diagnostic pop")
#else
#define BEGIN_DISABLE_WARNINGS
#define DISABLE_WARNING_EFFCPP
#define DISABLE_WARNING_STRICT_ALIASING
#define END_DISABLE_WARNINGS
#endif

#define _unused_ __attribute__ ((unused))

#define USEC_PER_SEC 1000000ULL
