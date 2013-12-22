#pragma once

#ifndef EMSCRIPTEN
// #define DODEBUG
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

static const char* getGlError(GLenum error)
{
#define casereturn(x) case x: return #x
	switch (error) {
			casereturn(GL_INVALID_ENUM);
			casereturn(GL_INVALID_VALUE);
			casereturn(GL_INVALID_OPERATION);
			casereturn(GL_OUT_OF_MEMORY);
		default:
			casereturn(GL_NO_ERROR);
	}
#undef casereturn
	return "";
}

#define GLDEBUG(x) \
	x; \
	{ \
		GLenum e; \
		while( (e=glGetError()) != GL_NO_ERROR) \
		{ \
			fprintf(stderr, "Error at line number %d, in file %s. glGetError() returned %s for call %s\n",__LINE__, __FILE__, getGlError(e), #x ); \
		} \
	}
#else
#define DEBUG(fmt)
#define DEBUGV(fmt, ...)
#define GLDEBUG(x) \
	x;
#endif
