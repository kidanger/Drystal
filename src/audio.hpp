#pragma once
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_mixer.h>
#include <queue>

#include "log.hpp"

class Audio
{
	public:
		Audio();
		~Audio();

		static void play_sound(const char *filepath);
		static void play_sound_queued(char *filepath);
	private:
		static void music_finished();

		static SDL_mutex *_mutex;
		static Mix_Music *_music;
		static std::queue<char *> _musicQueue;
};
