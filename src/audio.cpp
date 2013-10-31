#include <cassert>
#include <iostream>

#include "audio.hpp"

SDL_mutex *Audio::_mutex = SDL_CreateMutex();
Mix_Music *Audio::_music = NULL;
std::queue<char *> Audio::_musicQueue;

#define NUM_CHANNELS 64

Audio::Audio()
{
	int init_flags = MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG;
	int ret = Mix_Init(init_flags);
#ifndef EMSCRIPTEN
	if ((init_flags & ret) != init_flags)
		std::cerr << "[ERROR] cannot init audio: " << Mix_GetError() << std::endl;
#endif

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		std::cerr << "[ERROR] cannot open audio: " << Mix_GetError() << std::endl;

	assert(Mix_AllocateChannels(NUM_CHANNELS) == NUM_CHANNELS);
	Mix_HookMusicFinished(music_finished);
}

Audio::~Audio()
{
	while(Mix_Init(0))
		Mix_Quit();
	if (_music) {
		Mix_FreeMusic(_music);
		_music = NULL;
	}
	Mix_CloseAudio();
	SDL_DestroyMutex(_mutex);
}

void Audio::music_finished()
{
	SDL_mutexP(_mutex);
	if (!_musicQueue.empty())
	{
		SDL_mutexV(_mutex);
		play_music_queued(_musicQueue.front());
		SDL_mutexP(_mutex);
		_musicQueue.pop();
	}
	SDL_mutexV(_mutex);
}

void Audio::play_music_queued(char *filepath)
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

void Audio::play_music(const char *filepath, int times)
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

	if (Mix_PlayMusic(_music, times) != 0)
	{
		std::cerr << "cannot play music: `" << filepath << "': " << Mix_GetError() << std::endl;
	}
	SDL_mutexV(_mutex);
}

void Audio::stop_music()
{
	SDL_mutexP(_mutex);
	Mix_HaltMusic();
	if (_music)
		Mix_FreeMusic(_music);
	_music = NULL;
	SDL_mutexV(_mutex);
}

Mix_Chunk* Audio::load_sound(const char *filepath)
{
	Mix_Chunk *chunk = Mix_LoadWAV(filepath);
	if (chunk == NULL)
	{
		std::cerr << "[ERROR] cannot load sound file: `" << filepath << "': " << Mix_GetError() << std::endl;
	}
	return chunk;
}

Mix_Chunk* Audio::create_sound(unsigned int len, const float* buffer)
{
	uint16_t converted_buffer[len]; // 16bits per sample
	for (unsigned int i = 0; i < len; i++) {
		converted_buffer[i] = static_cast<uint16_t>(buffer[i] * 65535/2 + 65535/2);
	}
	Mix_Chunk *chunk = Mix_QuickLoad_RAW(reinterpret_cast<uint8_t*>(converted_buffer), len);
	return chunk;
}

void Audio::free_sound(Mix_Chunk *chunk)
{
	Mix_FreeChunk(chunk);
}

#ifdef EMSCRIPTEN
static int next_channel = 0;
#endif
void Audio::play_sound(Mix_Chunk *chunk, float volume)
{
	if (volume != -1 && volume < 0 && volume > 1)
	{
		fprintf(stderr, "[ERROR] cannot play sound: invalid volume value\n");
		return;
	}

	int channel = -1;
#ifdef EMSCRIPTEN
	// emscripten does not always handle channel assignation
	channel = next_channel % NUM_CHANNELS;
#endif
	if ((channel = Mix_PlayChannel(channel, chunk, 0)) == -1) {
		std::cerr << "[ERROR] cannot play sound: " << Mix_GetError() << std::endl;
		return;
	}
	if (volume != -1)
		Mix_Volume(channel, volume * MIX_MAX_VOLUME);
	else
		Mix_Volume(channel, Mix_Volume(-1, -1));
#ifdef EMSCRIPTEN
	next_channel = channel + 1;
#endif
}

void Audio::set_music_volume(float volume)
{
	if (volume < 0 && volume > 1)
	{
		fprintf(stderr, "[ERROR] cannot set music volume: invalid value\n");
		return;
	}
	Mix_VolumeMusic(volume * MIX_MAX_VOLUME);
}

void Audio::set_sound_volume(float volume)
{
	if (volume < 0 && volume > 1)
	{
		fprintf(stderr, "[ERROR] cannot set sound volume: invalid value\n");
		return;
	}
	Mix_Volume(-1, volume * MIX_MAX_VOLUME);
}

