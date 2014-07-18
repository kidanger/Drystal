#include "module.hpp"

BEGIN_MODULE(audio)
	DECLARE_FUNCTION(load_music)
	DECLARE_FUNCTION(set_music_volume)

	DECLARE_FUNCTION(load_sound)
	DECLARE_FUNCTION(create_sound)
	DECLARE_FUNCTION(set_sound_volume)

	BEGIN_CLASS(sound)
		ADD_METHOD(sound, play)
		ADD_GC(free_sound)
		END_CLASS()
	REGISTER_CLASS(sound, "Sound")

	BEGIN_CLASS(music)
		ADD_METHOD(music, play)
		ADD_METHOD(music, stop)
		ADD_GC(free_music)
		END_CLASS()
	REGISTER_CLASS(music, "Music")
END_MODULE()

