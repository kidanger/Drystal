#ifndef AUDIO_H
#define AUDIO_H

#include <AL/al.h>
#include <AL/alc.h>

struct Sound;
struct Music;

struct Source {
	ALuint alSource;
	bool used;
	bool isMusic;
	union {
		Sound* currentSound;
		Music* currentMusic;
	};
	float desiredVolume;
};

struct Sound {
	ALuint alBuffer;
};

struct Music {
	Source* source;
	ALuint alBuffer;
	int callback;
};

class Audio
{
	public:
		Audio();
		~Audio();

		void update(float dt);

		Sound* load_sound(const char *filepath);
		Sound* create_sound(unsigned int len, const float* buffer);
		void play_sound(Sound* sound, float volume=1, float x=0, float y=0);
		void free_sound(Sound* sound);

		Music* load_music(int callback);
		void play_music(Music* music);
		void stop_music(Music* music);

		void set_music_volume(float volume);
		void set_sound_volume(float volume);

	private:
		ALCcontext* context;
		ALCdevice* device;
		float globalSoundVolume;
		float globalMusicVolume;
};

#endif
