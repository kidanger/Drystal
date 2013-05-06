EXT=.o
EEXT=
ARGS=
EXEC=drystal$(EEXT)

DEBUG=yes

LUADIR=$(HOME)/dev/lua-5.2.2/install
#FREETYPEDIR=$(HOME)/dev/emscripten/tests/freetype

CC=clang++
CCFLAGS=-std=c++11 -I$(SRCDIR) -I$(LUADIR)/include -I$(HOME)/dev/sdl_oglblit-0.5/include #-I$(FREETYPEDIR)/include -v
CCFLAGS+=-Wall -Wextra

LD=clang++
SDL_OPTIONS=`sdl-config --libs` -lSDL_image -lSDL_ttf
LUA_OPTIONS=liblua.so
TTF_OPTIONS=libfreetype.bc.so libSDL_freetype.bc.so
#BLIT_OPTION=libSDL_oglblit.a
LDFLAGS+=$(SDL_OPTIONS) $(LUA_OPTIONS) #$(BLIT_OPTION) #$(TTF_OPTIONS)

SRCDIR=src
OBJDIR=obj

ifeq ($(PROF),yes)
	CCFLAGS+=-p
	CCFLAGS+=-g
endif

ifeq ($(DEBUG),yes)
	CCFLAGS+=-g
else
	CCFLAGS+=-O2
endif

WEB=
EMCC=emcc
ifneq ($(WEB),)
	CC=$(EMCC)
	LD=$(EMCC)
	EXT=.bc
	EEXT=.bc
	LDFLAGS+=$(shell cat included_files.txt) -s TOTAL_MEMORY=33554432 -s DEAD_FUNCTIONS="['_SDL_DisplayFormat']" --minify 1 -s ASM_JS=0 -O2 #-DNDEBUG
	EXEC=index.html
	SDL_OPTIONS=
	LUA_OPTIONS=liblua.bc.so
	TTF_OPTIONS=libfreetype.bc.so libSDL_freetype.bc.so
	#BLIT_OPTION=libSDL_oglblit.bc.so
endif

SRC:=$(shell find $(SRCDIR) -name "*.cpp")
OBJ:=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%$(EXT),$(SRC))

all: compile

compile: $(OBJDIR) depend $(EXEC)

run: compile
	./$(EXEC) $(ARGS)

debug: compile
	gdb -ex run $(EXEC) $(ARGS) -silent

valgrind: compile
	valgrind --db-attach=yes --leak-check=yes --tool=memcheck --num-callers=16 --leak-resolution=high ./$(EXEC) $(ARGS)

profile: clean
	make run PROF=yes
	prof $(EXEC) gmon.out > prof.out
	echo "Read the file \"prof.out\""

splint:
	splint $(SRC)

$(EXEC): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%$(EXT): $(SRCDIR)/%.cpp
	@mkdir -p `dirname $@`
	$(CC) -o $@ -c $< $(CCFLAGS)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	-rm .depend
	-rm -fr $(OBJDIR)
	-rm $(EXEC)

depend:
	-@makedepend -f- -Ysrc -- $(CCFLAGS) -- $(SRC) 2>/dev/null | \
		sed -e "s/^src\(.*\)/obj\1/" \
		>.depend

-include .depend
