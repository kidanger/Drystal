#ifndef WAVLOADER_HEADER
#define WAVLOADER_HEADER

#define WAV_FORMAT_U8 1
#define WAV_FORMAT_S8 2
#define WAV_FORMAT_U16 3
#define WAV_FORMAT_S16 4

#define WAV_CHANNEL_MONO 1
#define WAV_CHANNEL_STEREO 2

int load_wav(const char* filename, void** buffer, int* length, int* format, int* channels, int* samplerate);

#endif
