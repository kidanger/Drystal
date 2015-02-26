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

#ifdef BUILD_AUDIO
#define _AUDIO_FEATURE_ "+AUDIO"
#else
#define _AUDIO_FEATURE_ "-AUDIO"
#endif

#ifdef BUILD_FONT
#define _FONT_FEATURE_ "+FONT"
#else
#define _FONT_FEATURE_ "-FONT"
#endif

#ifdef BUILD_GRAPHICS
#define _GRAPHICS_FEATURE_ "+GRAPHICS"
#else
#define _GRAPHICS_FEATURE_ "-GRAPHICS"
#endif

#ifdef BUILD_LIVECODING
#define _LIVECODING_FEATURE_ "+LIVECODING"
#else
#define _LIVECODING_FEATURE_ "-LIVECODING"
#endif

#ifdef BUILD_PHYSICS
#define _PHYSICS_FEATURE_ "+PHYSICS"
#else
#define _PHYSICS_FEATURE_ "-PHYSICS"
#endif

#ifdef BUILD_PARTICLE
#define _PARTICLE_FEATURE_ "+PARTICLE"
#else
#define _PARTICLE_FEATURE_ "-PARTICLE"
#endif

#ifdef BUILD_STORAGE
#define _STORAGE_FEATURE_ "+STORAGE"
#else
#define _STORAGE_FEATURE_ "-STORAGE"
#endif

#ifdef BUILD_UTILS
#define _UTILS_FEATURE_ "+UTILS"
#else
#define _UTILS_FEATURE_ "-UTILS"
#endif

#ifdef BUILD_WEB
#define _WEB_FEATURE_ "+WEB"
#else
#define _WEB_FEATURE_ "-WEB"
#endif

#define DRYSTAL_FEATURES \
	_AUDIO_FEATURE_ " " \
	_FONT_FEATURE_ " " \
	_GRAPHICS_FEATURE_ " " \
	_LIVECODING_FEATURE_ " " \
	_PHYSICS_FEATURE_ " " \
	_PARTICLE_FEATURE_ " " \
	_STORAGE_FEATURE_ " " \
	_UTILS_FEATURE_ " " \
	_WEB_FEATURE_ " "
