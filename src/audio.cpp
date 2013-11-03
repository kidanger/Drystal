#include <cassert>
#include <cstdio>

#include "stb_vorbis.c"

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
		if (not source.used)
			continue;
		alGetSourcei(source.alSource, AL_SOURCE_STATE, &status);
		source.used = status == AL_PLAYING;

		if (source.used and source.isMusic) {
			Music* music = source.currentMusic;
			if (not music->ended) {
				stream_music(music);
			}
		}
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

Sound* Audio::create_sound(unsigned int len, const float* buffer, int samplesrate)
{
	ALushort converted_buffer[len]; // 16bits per sample
	for (unsigned int i = 0; i < len; i++) {
		converted_buffer[i] = static_cast<ALushort>(buffer[i] * 65535/2 + 65535/2);
	}

	Sound* sound = new Sound;
	alGenBuffers(1, &sound->alBuffer);
	alBufferData(sound->alBuffer, AL_FORMAT_MONO16,
			converted_buffer, len * sizeof(ALushort), samplesrate);

	error()
	return sound;
}

void Audio::free_sound(Sound* sound)
{
	alDeleteBuffers(1, &sound->alBuffer);
	error()
	delete sound;
}

static Source* get_free_source()
{
	for (int i = 0; i < NUM_SOURCES; i++) {
		if (not sources[i].used) {
			return &sources[i];
		}
	}
	fprintf(stderr, "no more source available\n");
	return NULL;
}

void Audio::play_sound(Sound* sound, float volume, float x, float y)
{
	Source* source = get_free_source();
	if (not source)
		return;

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

Music* Audio::load_music(MusicCallback* callback, int samplesrate, int num_channels)
{
	Music* music = new Music;
	music->callback = callback;
	music->samplesrate = samplesrate;
	music->buffersize = samplesrate * .1 * num_channels;
	music->format = num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	alGenBuffers(STREAM_NUM_BUFFERS, music->alBuffers);
	error()
	return music;
}

class VorbisMusicCallback : public MusicCallback {
	public:
		stb_vorbis* stream;
		stb_vorbis_info info;

		~VorbisMusicCallback()
		{
			stb_vorbis_close(stream);
		}

		unsigned int feed_buffer(unsigned short * buffer, unsigned int len)
		{
			int size = stb_vorbis_get_samples_short_interleaved(
									this->stream, this->info.channels,
									reinterpret_cast<short*>(buffer), len);
			size *= this->info.channels;
			return size;
		}
};
Music* Audio::load_music_from_file(const char* filename)
{
	VorbisMusicCallback* callback = new VorbisMusicCallback;

	callback->stream = stb_vorbis_open_filename((char*) filename, NULL, NULL);
	if (not callback->stream) {
		fprintf(stderr, "cannot load %s\n", filename);
		return NULL;
	}

	callback->info = stb_vorbis_get_info(callback->stream);

	return load_music(callback, callback->info.sample_rate, callback->info.channels);
}

void Audio::play_music(Music* music)
{
	Source* source = get_free_source();
	if (not source)
		return;

	ALushort buff[music->buffersize];
	unsigned int len;
	for (int i = 0; i < STREAM_NUM_BUFFERS; i++) {
		len = music->callback->feed_buffer(buff, music->buffersize);
		alBufferData(music->alBuffers[i], music->format, buff, len*sizeof(ALushort), music->samplesrate);
		error()
	}

	alSourceQueueBuffers(source->alSource, STREAM_NUM_BUFFERS, music->alBuffers);
	alSourcef(source->alSource, AL_GAIN, globalMusicVolume);
	alSourcePlay(source->alSource);
	error()

	music->source = source;
	music->ended = 0;
	source->isMusic = true;
	source->currentMusic = music;
	source->used = true;
	source->desiredVolume = 1;
}

void Audio::stream_music(Music* music)
{
	Source* source = music->source;

	ALint processed;
	alGetSourcei(source->alSource, AL_BUFFERS_PROCESSED, &processed);

	while(processed--)
	{
		ALuint buffer;
		alSourceUnqueueBuffers(source->alSource, 1, &buffer);
		error()

		ALushort buff[music->buffersize];
		unsigned int len = music->callback->feed_buffer(buff, music->buffersize);
		alBufferData(buffer, music->format, buff, len*sizeof(ALushort), music->samplesrate);
		error()

		if (len < music->buffersize) {
			music->ended = true;
		}

		alSourceQueueBuffers(source->alSource, 1, &buffer);
		error()
	}
}

void Audio::stop_music(Music* music)
{
	Source* source = music->source;
	alSourceStop(source->alSource);
	source->used = false;
}

void Audio::free_music(Music* music)
{
	if (music->source) {
		stop_music(music);
	}
	alDeleteBuffers(2, music->alBuffers);
	delete music->callback;
	delete music;
}

void Audio::set_music_volume(float volume)
{
	globalMusicVolume = volume;
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (source.isMusic) {
			alSourcef(source.alSource, AL_GAIN, source.desiredVolume*volume);
			error()
		}
	}
}

void Audio::set_sound_volume(float volume)
{
	globalSoundVolume = volume;
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (not source.isMusic) {
			alSourcef(source.alSource, AL_GAIN, source.desiredVolume*volume);
			error()
		}
	}
}

