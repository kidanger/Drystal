
Welcome to Drystal's User Guide!
================================

.. toctree::
    :maxdepth: 2

    api.rst

..    gettingstarted.rst
..    tutorial.rst
..    gamedistribution.rst

What is Drystal?
================

A game engine which allows developpers to program games in Lua.

Drystal can be run natively (at the moment, only GNU/Linux compilation has been tested) or through a browser.
Emscripten_ can compile the engine to Javascript. Browsers need WebGL capabilities.

.. and WebSockets if network is used in game.

Here is an example of a drystal game:

.. code-block:: lua

    local drystal = require 'drystal'
    
    function drystal.draw()
      drystal.set_color(200, 200, 200)
      drystal.draw_background()
    end

Features
--------

- **Window** creation: resizable or not, hide cursor
- **User events**: keyboard, mouse
- **Textures**: load various formats, render to texture support, ability to apply transformations (rotation, resizes)
- **Drawing primitives**: lines and triangles from the C++ code, extended by Lua with rectangles (filled or not) and circles
- **Audio** support: multiple musics, multiple sounds at once, loadable from file or from lua-generated buffers
- **Shader** support: GLSL

Some additional modules:

- **Truetype**: loads .ttf files and render. Using the given syntax, you can highlight words with color/size change.
- **Web**: adds a wget function to download content (ex: download sounds if needed) or run JS code
- **Physic**: uses Box2D to compute physic simulation (not a one-to-one binding)

.. - **Network**: TCP/WebSocket communication


Contribute
----------

- Source Code: http://github.com/drystal/drystal

Support
-------

If you are having issues, please let us know.
We have a mailing list located at: https://groups.google.com/forum/?hl=en#!forum/drystal

License
-------

Drystal is licensed under LGPLv3 license.

The external/ folder contains third-party libraries:

- stb_vorbis.c, stb_image.c and truetype.c which are in the public domain
- lua/ which is under the MIT License
- box2d/ which is under the zlib License
- lua-cjson/ which is under the MIT License
- wavloader.c which is in the public domain
- miniz.c which is in the public domain

.. _Emscripten: http://www.emscripten.org
