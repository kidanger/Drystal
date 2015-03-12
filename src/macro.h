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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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
#ifdef __clang__
#define BEGIN_DISABLE_WARNINGS \
	_Pragma("clang diagnostic push")

#define DISABLE_WARNING_EFFCPP \
	_Pragma("clang diagnostic ignored \"-Weffc++\"")
#define DISABLE_WARNING_STRICT_ALIASING \
	_Pragma("clang diagnostic ignored \"-Wstrict-aliasing\"")

#define END_DISABLE_WARNINGS \
	_Pragma("clang diagnostic pop")
#else
#define BEGIN_DISABLE_WARNINGS
#define DISABLE_WARNING_EFFCPP
#define DISABLE_WARNING_STRICT_ALIASING
#define END_DISABLE_WARNINGS
#endif
#endif

#define _malloc_ __attribute__ ((malloc))
#define _sentinel_ __attribute__ ((sentinel))
#define _unused_ __attribute__ ((unused))
#define _printf_(a,b) __attribute__ ((format (printf, a, b)))

#define USEC_PER_SEC 1000000ULL
#define MSEC_PER_SEC 1000ULL
#define NSEC_PER_USEC 1000ULL

#define MAX(a,b) \
	({ \
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; \
	})

/* Assert with Side Effects */
#ifdef NDEBUG
#define assert_se(x) (x)
#else
#define assert_se(x) assert(x)
#endif

#ifdef __cplusplus
}
#endif

