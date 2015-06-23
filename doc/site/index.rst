
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
- **Shader** support: GLSL and post processing effect facilities

Some additional modules:

- **Font**: loads .ttf files and render. Using the given syntax, you can highlight words with color/size change.
- **Web**: adds a wget function to download content (ex: download sounds if needed) or run JS code
- **Physics**: uses Box2D to compute physics simulation (not a one-to-one binding)
- **Particle** systems to easily improve the game aspect
- **Storage** to save/load games (even in browser)


Contribute
----------

- Source Code: http://github.com/kidanger/drystal
- Please report any issue here: http://github.com/kidanger/drystal/issues


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

.. _Emscripten: http://www.emscripten.org
