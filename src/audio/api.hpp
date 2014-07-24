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
#include "audio_bind.hpp"
#include "music_bind.hpp"
#include "sound_bind.hpp"

BEGIN_MODULE(audio)
	DECLARE_FUNCTION(load_music)
	DECLARE_FUNCTION(set_music_volume)

	DECLARE_FUNCTION(load_sound)
	DECLARE_FUNCTION(set_sound_volume)

	BEGIN_CLASS(sound)
		ADD_METHOD(sound, play)
		ADD_GC(free_sound)
	REGISTER_CLASS(sound, "Sound")

	BEGIN_CLASS(music)
		ADD_METHOD(music, play)
		ADD_METHOD(music, stop)
		ADD_GC(free_music)
	REGISTER_CLASS(music, "Music")
END_MODULE()

