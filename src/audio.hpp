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

		Mix_Chunk *load_sound(const char *filepath);
		void free_sound(Mix_Chunk *chunk);
		void play_sound(Mix_Chunk *chunk);

		static void play_music(const char *filepath);
		static void play_music_queued(char *filepath);
	private:
		static void music_finished();

		static SDL_mutex *_mutex;
		static Mix_Music *_music;
		static std::queue<char *> _musicQueue;
};
