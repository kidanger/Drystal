/* wavloader - public domain wav loader
                                     no warranty implied; use at your own risk
* written in 2013 by kidanger
*/
#ifndef WAVLOADER_INCLUDE
#define WAVLOADER_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

int load_wav(const char* filename, void** buffer, int* length, int* format, int* channels, int* samplerate);

#ifdef __cplusplus
}
#endif

#endif // WAVLOADER_INCLUDE

#ifndef WAVLOADER_HEADER_ONLY
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int load_wav(const char* filename, void** buffer, int* length, int* format, int* channels, int* samplerate)
{
	/* see https://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */
	FILE* file = fopen(filename, "rb");
	unsigned int filesize;

	int errno = 1;
#define SKIP(n) { char skip[n]; fread(skip, n, 1, file); }
#define READ_STR(n, correct) { \
		char str[n]; \
		fread(str, n, 1, file); \
		if (strncmp(str, correct, n)) { \
			printf("failed to read %s was %s.\n", correct, str); \
			return errno; \
		} \
		errno++; \
	}
#define READ_NUM(type, correct) { \
		type num; \
		fread(&num, sizeof(num), 1, file); \
		if (num != correct) { \
			printf("failed to read %d was %d\n", correct, num); \
			return errno; \
		} \
		errno++; \
	}
	READ_STR(4, "RIFF"); /* ChunkID */
	fread(&filesize, sizeof(filesize), 1, file); /* ChunkSize */
	READ_STR(4, "WAVE"); /* Format */
	READ_STR(4, "fmt "); /* Subchunk1ID */
	SKIP(4); /* Subchunk1Size */
	SKIP(2); /* AudioFormat */

	short _channels;
	fread(&_channels, 2, 1, file);
	*channels = _channels;

	fread(samplerate, 4, 1, file);

	SKIP(4); /* ByteRate */
	SKIP(2); /* BlockAlign */
	READ_NUM(int16_t, 16); /* BitsPerSample */
	*format = 16; /* force 16bits per sample */
	READ_STR(4, "data");

	fread(length, 4, 1, file);
	*length /= sizeof(unsigned short);

	*buffer = malloc(*length * sizeof(unsigned short));
	int read = fread(*buffer, sizeof(unsigned short), *length, file);

	if (*length != read)
		return errno;

#undef SKIP
#undef READ_STR
#undef READ_NUM
	return 0;
	}

#ifdef __cplusplus
}
#endif

#ifdef WAVLOADER_MAIN
int main(int argc, const char** argv)
{
	void* buffer;
	int len, format, channels, samplerate;
	int ret = load_wav(argv[1], &buffer, &len, &format, &channels, &samplerate);
	printf("%d %d %d %d %d %p\n", ret, len, format, channels, samplerate, buffer);
	return 0;
}
#endif

#endif

