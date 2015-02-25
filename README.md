Drystal
=======

About
-----

Drystal is an opensource game engine.

Games are programmed in Lua and can be exported to Javascript seamlessly.

Authors
-------

Jérémy Anger
Ronny Chevalier

Mailing list
------------

https://groups.google.com/forum/#!forum/drystal

To subscribe, send an e-mail to:

    drystal+subscribe@googlegroups.com

To unsubscribe, send an e-mail to:

    drystal+unsubscribe@googlegroups.com

To follow via RSS:

    https://groups.google.com/forum/feed/drystal/topics/rss.xml

To follow via ATOM:

    https://groups.google.com/forum/feed/drystal/topics/atom.xml

License
-------

LGPLv3 for all Drystal code

The external/ folder contains third-party libraries:

- stb\_vorbis.c and stb\_truetype.h which are in the public domain
- lua/\* which is under the MIT License
- box2d/* which is under the zlib License
- lua-cjson/\* which is under the MIT License
- wavloader.c which is in the public domain
- miniz.c which is in the public domain
- libpng/\* which is under the libpng License
- zlib/\* which is under the zlib License

Repository
----------

The main repository of Drystal is hosted on Github at http://github.com/kidanger/drystal/

You can clone this repository with the following command

    git clone --recursive https://github.com/kidanger/drystal.git

Documentation
-------------

The documentation of the current release is available in HTML format and can be found at http://drystal.github.io/

Requirements
------------

To build Drystal, the following tools are needed:

- CMake
- C compiler — Currently, only clang and gcc are supported.
- make or ninja — Or any generators supported by CMake
- C++ compiler (optional)
- patch (optional)

And the following optional libraries:

- SDL2
- OpenAL
- OpenGL
- libpng >= 1.6
- Box2D

When building the documentation, the following additional dependencies are needed:

- sphinx
- pip

When building a web version of Drystal, you also need [Emscripten](kripken.github.io/emscripten-site/).

To use the test coverage, the following tools are needed:

- gcov
- lcov

First time
----------

### Native build

Once you cloned the repository, you can build Drystal in native mode (i.e.
that will run on your PC).

First, you need to create a build directory (this is optionnal but it leaves
the source directory clean):

    mkdir build-native-release
    cd build-native-release

Then, you run CMake that configures the build and creates Makefiles:

    cmake .. -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release

If everything is ok, CMake tells you that all build files have been written.
Otherwise, it may fail because of missing dependencies (e.g. libpng was not
found), so you need to fix this and re-run CMake.

Finally, you run make:

    make

You can now find the executable in the src/ directory

    src/drystal --help

### Web build

In order to compile Drystal to javascript you will need Emscripten and the
$EMSCRIPTEN environment variable should contain the path to your Emscripten
installation (e.g. /usr/lib/emscripten/).

First, you need to create a separate build directory:

    mkdir build-web-release
    cd build-web-release

Then, you run CMake that configures the build to use Emscripten and creates Makefiles:

    cmake .. -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake -DBUILD_LIVECODING=NO

Finally, you can run make:

    make

You will find the final javascript code (there will be two files drystal.js
and drystal.mem.js) in the src/ directory.

Build options
-------------

You can configure the build to fit to your needs, e.g. you do not need
the physics engine, you need to add -DBUILD_PHYSICS=OFF

Here is the list of options, their default and the additional dependencies needed to build:

Option                | Default | Additional dependencies
----------------------|---------|------------------------
BUILD_MANPAGES        | ON      | xsltproc
BUILD_ENABLE_COVERAGE | OFF     | lcov, gcov
BUILD_LIVECODING      | ON      |
BUILD_PHYSICS         | ON      | Box2D
BUILD_PARTICLE        | ON      |
BUILD_WEB             | ON      |
BUILD_FONT            | ON      | SDL2, OpenGL
BUILD_AUDIO           | ON      | OpenAL
BUILD_STORAGE         | ON      |
BUILD_GRAPHICS        | ON      | SDL2, OpenGL, libpng
BUILD_UTILS           | ON      |

The additional dependencies listed here are only for a native build. When
building with Emscripten, all these dependencies are either provided
by Emscripten or recompiled entirely (using a git submodule).

For the web build, removing parts of Drystal that you do not use decrease
the size of the final javascript code which helps loading the page of the
game faster. (e.g. removing the physics module saves ~273 KiB)

Documentation build
-------------------

First, you need sphinx and pip (python2) then, as root, run:

    pip install sphinx_rtd_theme

If you want to build the documentation in HTML go to doc/site/ and run:

    make html
