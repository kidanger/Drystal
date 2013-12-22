#include <cassert>
#include <cstdio>

#include "stb_vorbis.c"
#include "wavloader.c"

#include "audio.hpp"

#define NUM_SOURCES 16

#define error() do { \
		ALint error; \
		while ((error = alGetError()) != AL_NO_ERROR) { \
			fprintf(stderr, "[ALerr] %s:%d %s\n", __func__, __LINE__, getAlError(error)); \
		} \
	} while (false)

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

#define INIT_IF_NEEDED(ret) do { \
		if (!initialized) \
			init(); \
		if (!initialized) \
			return ret; \
	} while (false)

Source sources[NUM_SOURCES];

Audio::Audio() :
	initialized(false),
	context(NULL),
	device(NULL),
	globalSoundVolume(1.),
	globalMusicVolume(1.)
{
}

Audio::~Audio()
{
	if (initialized) {
		for (int i = 0; i < NUM_SOURCES; i++)
			alDeleteSources(1, &sources[i].alSource);

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
}

bool Audio::init()
{
	device = alcOpenDevice(NULL);
	if (!device) {
		fprintf(stderr, "cannot open device\n");
		return false;
	}

	context = alcCreateContext(device, NULL);
	if (!context) {
		fprintf(stderr, "cannot create context\n");
		return false;
	}

	if (!alcMakeContextCurrent(context)) {
		fprintf(stderr, "cannot make context\n");
		return false;
	}

	for (int i = 0; i < NUM_SOURCES; i++)
		alGenSources(1, &sources[i].alSource);

	initialized = true;
	return true;
}

void Audio::update(float dt)
{
	(void)dt;
	if (!initialized)
		return;

	ALint status;
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (!source.used)
			continue;
		alGetSourcei(source.alSource, AL_SOURCE_STATE, &status);
		source.used = status == AL_PLAYING;

		if (!source.used && !source.isMusic) { // ended sound
			Sound* sound = source.currentSound;
			if (sound->free_me) {
				free_sound(sound);
			}
		}
		if (source.used && source.isMusic) { // still playing music
			Music* music = source.currentMusic;
			if (!music->ended) {
				stream_music(music);
			}
		}
	}
}

Sound* Audio::load_sound(const char *filepath)
{
	INIT_IF_NEEDED(NULL);

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

	Sound* sound = new Sound;

	sound->free_me = false;
	alGenBuffers(1, &sound->alBuffer);
	alBufferData(sound->alBuffer, AL_FORMAT_MONO16,
	             buffer, length * sizeof(ALushort), samplerate);

	error();
	return sound;
}

Sound* Audio::create_sound(unsigned int len, const float* buffer, int samplesrate)
{
	INIT_IF_NEEDED(NULL);
	ALushort converted_buffer[len]; // 16bits per sample
	for (unsigned int i = 0; i < len; i++) {
		converted_buffer[i] = static_cast<ALushort>(buffer[i] * 65535 / 2 + 65535 / 2);
	}

	Sound* sound = new Sound;
	sound->free_me = false;
	alGenBuffers(1, &sound->alBuffer);
	alBufferData(sound->alBuffer, AL_FORMAT_MONO16,
	             converted_buffer, len * sizeof(ALushort), samplesrate);

	error();
	return sound;
}

void Audio::free_sound(Sound* sound)
{
	bool can_free = true;
	// stop sources which used the sound
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (source.currentSound == sound) {
			if (source.used) {
				can_free = false;
			} else {
				alSourceStop(source.alSource);
				alSourcei(source.alSource, AL_BUFFER, 0);
			}
		}
	}
	error();
	// if there's no more source playing the sound, free it
	if (can_free) {
		alDeleteBuffers(1, &sound->alBuffer);
		delete sound;
		error();
	} else {
		// otherwise, just delay the deletion
		sound->free_me = true;
	}
}

static Source* get_free_source()
{
	for (int i = 0; i < NUM_SOURCES; i++) {
		if (!sources[i].used) {
			return &sources[i];
		}
	}
	fprintf(stderr, "no more source available\n");
	return NULL;
}

void Audio::play_sound(Sound* sound, float volume, float x, float y)
{
	if (!sound)
		// sound is not loaded properly
		return;

	Source* source = get_free_source();
	if (!source)
		return;

	error();
	alSourcei(source->alSource, AL_BUFFER, sound->alBuffer);
	error();
	alSource3f(source->alSource, AL_POSITION, x, y, 0.);
	error();
	alSourcef(source->alSource, AL_GAIN, volume * globalSoundVolume);
	error();
	alSourcePlay(source->alSource);
	error();

	source->isMusic = false;
	source->currentSound = sound;
	source->used = true;
	source->desiredVolume = volume;
}

Music* Audio::load_music(MusicCallback* callback, int samplesrate, int num_channels)
{
	INIT_IF_NEEDED(NULL);
	Music* music = new Music;
	music->callback = callback;
	music->samplesrate = samplesrate;
	music->buffersize = samplesrate * .1 * num_channels;
	music->format = num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	alGenBuffers(STREAM_NUM_BUFFERS, music->alBuffers);
	error();
	return music;
}

class VorbisMusicCallback : public MusicCallback
{
public:
	stb_vorbis* stream;
	stb_vorbis_info info;

	~VorbisMusicCallback() {
		stb_vorbis_close(stream);
	}

	unsigned int feed_buffer(unsigned short * buffer, unsigned int len) {
		int size = stb_vorbis_get_samples_short_interleaved(
		               this->stream, this->info.channels,
		               reinterpret_cast<short*>(buffer), len);
		size *= this->info.channels;
		return size;
	}
};
Music* Audio::load_music_from_file(const char* filename)
{
	INIT_IF_NEEDED(NULL);
	VorbisMusicCallback* callback = new VorbisMusicCallback;

	callback->stream = stb_vorbis_open_filename((char*) filename, NULL, NULL);
	if (!callback->stream) {
		fprintf(stderr, "cannot load %s\n", filename);
		return NULL;
	}

	callback->info = stb_vorbis_get_info(callback->stream);

	return load_music(callback, callback->info.sample_rate, callback->info.channels);
}

void Audio::play_music(Music* music)
{
	if (!music)
		// music is not loaded properly
		return;

	Source* source = get_free_source();
	if (!source)
		return;

	ALushort buff[music->buffersize];
	unsigned int len;
	for (int i = 0; i < STREAM_NUM_BUFFERS; i++) {
		len = music->callback->feed_buffer(buff, music->buffersize);
		alBufferData(music->alBuffers[i], music->format, buff, len * sizeof(ALushort), music->samplesrate);
		error();
	}

	alSourceQueueBuffers(source->alSource, STREAM_NUM_BUFFERS, music->alBuffers);
	alSourcef(source->alSource, AL_GAIN, globalMusicVolume);
	alSourcePlay(source->alSource);
	error();

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

	while (processed--) {
		ALuint buffer;
		alSourceUnqueueBuffers(source->alSource, 1, &buffer);
		error();

		ALushort buff[music->buffersize];
		unsigned int len = music->callback->feed_buffer(buff, music->buffersize);
		alBufferData(buffer, music->format, buff, len * sizeof(ALushort), music->samplesrate);
		error();

		if (len < music->buffersize) {
			music->ended = true;
		}

		alSourceQueueBuffers(source->alSource, 1, &buffer);
		error();
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
	if (!initialized)
		return;

	// update current playing musics
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (source.isMusic) {
			alSourcef(source.alSource, AL_GAIN, source.desiredVolume * volume);
			error();
		}
	}
}

void Audio::set_sound_volume(float volume)
{
	globalSoundVolume = volume;
	if (!initialized)
		return;

	// update current playing sounds
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (!source.isMusic) {
			alSourcef(source.alSource, AL_GAIN, source.desiredVolume * volume);
			error();
		}
	}
}

