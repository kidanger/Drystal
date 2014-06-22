#pragma once

#ifndef EMSCRIPTEN
#define DODEBUG
#endif

#ifdef DODEBUG
#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif
#include <cstdio>

#define DEBUG(fmt)\
	do { fprintf(stderr, "%10.10s:%d\t%s()\t" fmt "\n", __FILE__, \
		             __LINE__, __func__); } while (0)
#define DEBUGV(fmt, ...)\
	do { fprintf(stderr, "%10.10s:%d\t%s()\t" fmt "\n", __FILE__, \
		             __LINE__, __func__, __VA_ARGS__); } while (0)

const char* getGLError(GLenum error);

#define GLDEBUG(x) \
	x; \
	{ \
		GLenum e; \
		while( (e=glGetError()) != GL_NO_ERROR) \
		{ \
			fprintf(stderr, "Error at line number %d, in file %s. glGetError() returned %s for call %s\n",__LINE__, __FILE__, getGLError(e), #x ); \
			exit(1); \
		} \
	}
#else
#define DEBUG(fmt)
#define DEBUGV(fmt, ...)
#define GLDEBUG(x) \
	x;
#endif
