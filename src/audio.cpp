#include <cassert>
#include <cstdio>

#include "audio.hpp"
#include "wavloader.hpp"

#define NUM_SOURCES 16

#define error() \
{ \
	ALint error; \
	while ((error = alGetError()) != AL_NO_ERROR) { \
		fprintf(stderr, "[ALerr] %s:%d %s\n", __func__, __LINE__, getAlError(error)); \
	} \
}
const char* getAlError(ALint error)
{
#define casereturn(x) case x: return #x
	switch (error) {
		casereturn(AL_INVALID_NAME);
		casereturn(AL_INVALID_ENUM);
		casereturn(AL_INVALID_VALUE);
		casereturn(AL_INVALID_OPERATION);
		casereturn(AL_OUT_OF_MEMORY);
		default:
		casereturn(AL_NO_ERROR);
	}
#undef casereturn
	return "";
}

Source sources[NUM_SOURCES];

Audio::Audio() :
	globalSoundVolume(1.),
	globalMusicVolume(1.)
{
	device = alcOpenDevice(NULL);
	if (!device)
		fprintf(stderr, "cannot open device\n");

	context = alcCreateContext(device, NULL);
	if (!context)
		fprintf(stderr, "cannot create context\n");

	if (!alcMakeContextCurrent(context))
		fprintf(stderr, "cannot make context\n");
	for (int i = 0; i < NUM_SOURCES; i++)
		alGenSources(1, &sources[i].alSource);
}

Audio::~Audio()
{
	for (int i = 0; i < NUM_SOURCES; i++)
		alDeleteSources(1, &sources[i].alSource);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void Audio::update(float dt)
{
	(void)dt;
	ALint status;
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		alGetSourcei(source.alSource, AL_SOURCE_STATE, &status);
		source.used = status == AL_PLAYING;
	}
}

Sound* Audio::load_sound(const char *filepath)
{
	Sound* sound = new Sound;

	void* buffer;
	int length;
	int format;
	int channels;
	int samplerate;
	int err = load_wav(filepath, &buffer, &length, &format, &channels, &samplerate);
	if (err) {
		fprintf(stderr, "could not load %s\n", filepath);
		return NULL;
	}

	alGenBuffers(1, &sound->alBuffer);
	alBufferData(sound->alBuffer, AL_FORMAT_MONO16,
			buffer, length * sizeof(ALushort), samplerate);

	error()
	return sound;
}

Sound* Audio::create_sound(unsigned int len, const float* buffer)
{
	ALushort converted_buffer[len]; // 16bits per sample
	for (unsigned int i = 0; i < len; i++) {
		converted_buffer[i] = static_cast<ALushort>(buffer[i] * 65535/2 + 65535/2);
	}

	Sound* sound = new Sound;
	alGenBuffers(1, &sound->alBuffer);
	alBufferData(sound->alBuffer, AL_FORMAT_MONO16,
			converted_buffer, len * sizeof(ALushort), 44100);

	error()
	return sound;
}

void Audio::free_sound(Sound* sound)
{
	alDeleteBuffers(1, &sound->alBuffer);
	error()
	delete sound;
}

void Audio::play_sound(Sound* sound, float volume, float x, float y)
{
	Source* source = NULL;

	for (int i = 0; i < NUM_SOURCES; i++) {
		if (not sources[i].used) {
			source = &sources[i];
			break;
		}
	}
	if (not source) {
		fprintf(stderr, "no more source available\n");
		return;
	}

	error()
	alSourcei(source->alSource, AL_BUFFER, sound->alBuffer);
	error()
	alSource3f(source->alSource, AL_POSITION, x, y, 0.);
	error()
	alSourcef(source->alSource, AL_GAIN, volume*globalSoundVolume);
	error()
	alSourcePlay(source->alSource);
	error()

	source->isMusic = false;
	source->currentSound = sound;
	source->used = true;
	source->desiredVolume = volume;
}

void Audio::play_music(Music* music)
{
//	alSourcei(alSourceID, AL_SOURCE_RELATIVE, AL_TRUE);
//	alSource3f(alSourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
}

void Audio::stop_music(Music* music)
{
}

void Audio::set_music_volume(float volume)
{
	globalMusicVolume = volume;
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (source.isMusic) {
			alSourcei(source.alSource, AL_GAIN, source.desiredVolume*volume);
		}
	}
}

void Audio::set_sound_volume(float volume)
{
	globalSoundVolume = volume;
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (not source.isMusic) {
			alSourcei(source.alSource, AL_GAIN, source.desiredVolume*volume);
		}
	}
}

