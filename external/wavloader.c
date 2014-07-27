/* wavloader - public domain wav loader no warranty implied; use at your own risk
* written in 2013 by kidanger
* rewritten in 2014 by Ronny Chevalier
*/
#ifndef WAVLOADER_INCLUDE
#define WAVLOADER_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

struct wave_header {
	char header_id[4];
	uint32_t chunk_size;
	char format[4];
	char format_id[4];
	uint32_t format_size;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	char data_id[4];
	uint32_t data_size;
};

int load_wav(const char *filename, struct wave_header *wave_header, void **audio_data);

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
#include <errno.h>

int load_wav(const char *filename, struct wave_header *wave_header, void **audio_data)
{
	/* see https://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */
	FILE *file;
	void *tmp_buffer = NULL;
	int ret = 0;
	size_t r;
	long filesize;

	if (!filename || !wave_header || !audio_data) {
		return -EINVAL;
	}

	file = fopen(filename, "rb");
	if (!file) {
		return -errno;
	}

	r = fread(wave_header, sizeof(struct wave_header), 1, file);
	if (r != 1) {
		ret = -EIO;
		goto fail;
	}

	if (strncmp(wave_header->header_id, "RIFF", 4)) {
		ret = -ENOTSUP;
		goto fail;
	}
	if (strncmp(wave_header->format, "WAVE", 4)) {
		ret = -ENOTSUP;
		goto fail;
	}
	if (strncmp(wave_header->format_id, "fmt", 3)) {
		ret = -ENOTSUP;
		goto fail;
	}
	if (strncmp(wave_header->data_id, "data", 4)) {
		ret = -ENOTSUP;
		goto fail;
	}
	if (wave_header->format_size != 16) {
		ret = -ENOTSUP;
		goto fail;
	}
	if (wave_header->audio_format != 1) {
		ret = -ENOTSUP;
		goto fail;
	}

	fseek(file, 0L, SEEK_END);
	filesize = ftell(file);
	fseek(file, sizeof(struct wave_header), SEEK_SET);

	if (wave_header->data_size != filesize - sizeof(struct wave_header)) {
		ret = -EIO;
		goto fail;
	}

	tmp_buffer = malloc(wave_header->data_size);
	if (!tmp_buffer) {
		ret = -ENOMEM;
		goto fail;
	}

	r = fread(tmp_buffer, wave_header->data_size, 1, file);
	if (r != 1) {
		ret = -EIO;
		goto fail;
	}

	*audio_data = tmp_buffer;
	fclose(file);

	return ret;
fail:
	free(tmp_buffer);
	fclose(file);

	return ret;
}
#ifdef __cplusplus
}
#endif
#endif

