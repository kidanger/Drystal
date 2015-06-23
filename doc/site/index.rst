
Welcome to Drystal's website!
================================

.. toctree::
    :maxdepth: 2

    api.rst
    tutorials.rst
    resources.rst

..    gettingstarted.rst
..    tutorial.rst
..    gamedistribution.rst

What is Drystal?
================

Drystal is a Lua 2D game engine.
It is free and open-source.

The engine is compiled to Javascript thanks to Emscripten_.
Games can be ran on Linux or on any platform with a recent web browser.

You can find a game made with Drystal `here <http://kidanger.github.io/Chronored>`_. It has been made in 48h during a
`Ludum Dare <http://ludumdare.com/compo/>`_.


Features
--------

- **Window** creation: hide cursor and relative mode
- **User events**: keyboard, mouse
- **Textures**: load PNGs, render to texture support, ability to apply transformations (rotation, resizes)
- **Drawing primitives**: lines, triangles, rectangles (filled or not), circles and polygons
- **Audio** support: multiple musics, multiple sounds at once, loadable from file or from lua-generated buffers
- **Shader** support: GLSL and post processing effect facilities (e.g. distortion, pixelation, blur,...)
- **Livecoding**: see instantly the modifications done in your game on the screen without restarting it

Some additional modules:

- **Font**: loads TTF files and display text. Using the given syntax, you can highlight words with color/size change.
- **Web**: downloads content from the web (ex: download sounds if needed) or runs javascript code
- **Physics**: uses Box2D to compute physics simulation (not a one-to-one binding)
- **Particle**: renders particles (e.g. smoke, fire,...) to easily improve the game aspect
- **Storage**: saves and loads data (e.g. game saves)

How to get Drystal?
-------------------

In order to build Drystal you need to install the following packages on your system:

- cmake
- gcc
- make
- patch
- git
- libpng
- openal
- box2d
- sdl2

Then, you need to get Drystal source code::

    git clone --recursive http://github.com/kidanger/Drystal.git
    cd Drystal

Then, you run CMake that configures the build::

    cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr

If everything is ok, CMake tells you that all build files have been written.
Otherwise, it may fail because of missing dependencies (e.g. libpng was not
found), so you need to fix this and re-run CMake.

Finally, you run make to build Drystal and make install to install it::

    make
    sudo make install

You can now use Drystal::

    drystal --help

Contribute
----------

- Source Code: http://github.com/kidanger/Drystal
- Please report any issue here: http://github.com/kidanger/Drystal/issues


Support
-------

If you are having trouble using Drystal, please let us know.
We have a mailing list located at: https://groups.google.com/forum/#!forum/drystal


License
-------

Drystal is licensed under LGPLv3 license.

The external/ folder contains third-party libraries:

- stb_vorbis.c and stb_truetype.c which are in the public domain
- lua/ which is under the MIT License
- box2d/ which is under the zlib License
- lua-cjson/ which is under the MIT License
- wavloader.c which is in the public domain
- miniz.c which is in the public domain
- libpng/\* which is under the libpng License
- zlib/\* which is under the zlib License

.. _Emscripten: http://www.emscripten.org
