EXT=.o
EEXT=
ARGS=
EXEC=drystal$(EEXT)

DEBUG=no

LUADIR=$(HOME)/dev/lua-5.2.2/install

CC=clang++
CCFLAGS=-std=c++11 -I$(SRCDIR) -I$(LUADIR)/include
CCFLAGS+=-Wall -Wextra

LD=clang++
SDL_OPTIONS=`sdl-config --libs` -lSDL_image -lSDL_ttf -lSDL_gfx
LUA_OPTIONS=-llua
LDFLAGS+=$(SDL_OPTIONS) $(LUA_OPTIONS)

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
	CC=$(EMCC) -v
	LD=$(EMCC) -v
	EXT=.bc
	EEXT=.bc
	DIRCOMP=$(HOME)/dev/emscripten/third_party
	LDFLAGS+=--preload-file data --minify 1 -s ASM_JS=1 -O2  --compression $(DIRCOMP)/lzma.js/lzma-native,$(DIRCOMP)/lzma.js/lzma-decoder.js,LZMA.decompress
	EXEC=index.html
	SDL_OPTIONS=
	LUA_OPTIONS=liblua.bc.so
endif

SRC:=$(shell find $(SRCDIR) -name "*.cpp")
OBJ:=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%$(EXT),$(SRC))

all: compile

compile: $(OBJDIR) depend $(EXEC) server

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

runserver:
	python server/Server.py

depend:
	-@makedepend -f- -Ysrc -- $(CCFLAGS) -- $(SRC) 2>/dev/null | \
		sed -e "s/^src\(.*\)/obj\1/" \
		>.depend

-include .depend
