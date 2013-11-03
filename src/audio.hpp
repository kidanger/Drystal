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

class MusicCallback {
	public:
		virtual ~MusicCallback() {}
		virtual unsigned int feed_buffer(unsigned short * buffer, unsigned int len) = 0;
};

#define STREAM_NUM_BUFFERS 3
struct Music {
	Source* source;
	ALuint alBuffers[STREAM_NUM_BUFFERS];
	MusicCallback* callback;
	ALenum format;
	int samplesrate;
	unsigned int buffersize;
	bool ended;
};

#define DEFAULT_SAMPLES_RATE 44100

class Audio
{
	public:
		Audio();
		~Audio();

		void update(float dt);

		Sound* load_sound(const char *filepath);
		Sound* create_sound(unsigned int len, const float* buffer, int samplesrate=DEFAULT_SAMPLES_RATE);
		void play_sound(Sound* sound, float volume=1, float x=0, float y=0);
		void free_sound(Sound* sound);

		Music* load_music(MusicCallback* callback, int samplesrate=DEFAULT_SAMPLES_RATE, int num_channels=1);
		Music* load_music_from_file(const char* filename);
		void play_music(Music* music);
		void stop_music(Music* music);
		void free_music(Music* music);

		void set_music_volume(float volume);
		void set_sound_volume(float volume);

	private:
		ALCcontext* context;
		ALCdevice* device;
		float globalSoundVolume;
		float globalMusicVolume;

		void stream_music(Music* music);
};

#endif
