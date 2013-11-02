#include <cstdio>
#include <cstring>

int load_wav(const char* filename, void** buffer, int* length, int* format, int* channels, int* samplerate)
{
	FILE* file = fopen(filename, "rb");
	char tag[4];
	unsigned int filesize;
	char filetype[4];
	char fmt[4];

	fread(tag, 4, 1, file);
	if (strcmp(tag, "TIFF")) {
		return 1;
	}
	fread(&filesize, sizeof(filesize), 1, file);
	fread(filetype, 4, 1, file);
	if (strcmp(filetype, "WAVE")) {
		return 1;
	}
	fread(fmt, 4, 1, file);
	return 0;
	// TODO
}

