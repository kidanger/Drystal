#include <iostream>

#include "audio.hpp"

SDL_mutex *Audio::_mutex = SDL_CreateMutex();
Mix_Music *Audio::_music = NULL;
std::queue<char *> Audio::_musicQueue;

Audio::Audio()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
		std::cerr << "[ERROR] open audio: " << Mix_GetError() << std::endl;
	Mix_HookMusicFinished(music_finished);
}

Audio::~Audio()
{
	if (_music)
		Mix_FreeMusic(_music);
	Mix_CloseAudio();
	SDL_DestroyMutex(_mutex);
}

void Audio::music_finished()
{
	SDL_mutexP(_mutex);
	if (!_musicQueue.empty())
	{
		play_sound_queued(_musicQueue.front());
		_musicQueue.pop();
	}
	SDL_mutexV(_mutex);
}

void Audio::play_sound_queued(char *filepath)
{
	SDL_mutexP(_mutex);
	if (Mix_PlayingMusic() != 0)
	{
		_musicQueue.push(filepath);
		SDL_mutexV(_mutex);
		return;
	}

	if (_music)
		Mix_FreeMusic(_music);

	_music = Mix_LoadMUS(filepath);
	if (_music == NULL)
	{
		std::cerr << "cannot load music: `" << filepath << "': " << Mix_GetError() << std::endl;
		free(filepath);
		SDL_mutexV(_mutex);
		return;
	}

	if (Mix_PlayMusic(_music, 1) != 0)
	{
		std::cerr << "cannot play music: `" << filepath << "': " << Mix_GetError() << std::endl;
	}
	free(filepath);
	SDL_mutexV(_mutex);
}

void Audio::play_sound(const char *filepath)
{
	SDL_mutexP(_mutex);
	Mix_HaltMusic();

	if (_music)
		Mix_FreeMusic(_music);

	_music = Mix_LoadMUS(filepath);
	if (_music == NULL)
	{
		std::cerr << "cannot load music: `" << filepath << "': " << Mix_GetError() << std::endl;
		SDL_mutexV(_mutex);
		return;
	}

	if (Mix_PlayMusic(_music, 1) != 0)
	{
		std::cerr << "cannot play music: `" << filepath << "': " << Mix_GetError() << std::endl;
	}
	SDL_mutexV(_mutex);
}
