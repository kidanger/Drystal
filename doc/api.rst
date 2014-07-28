.. highlightlang:: lua
   :linenothreshold: 5

.. lua:module:: drystal

.. role:: lua(code)
   :language: lua

API Reference
=============

Engine
------

.. _drystal-stop:
.. lua:function:: stop()

   Stop execution of drystal.

.. _drystal-reload:
.. lua:function:: reload()

   Reload the current game. This can be useful during the development in native build.

   .. note:: F3 also reloads the game.


Callbacks
^^^^^^^^^

.. lua:function:: init()

   This function is called after script loading, before the first frame.
   You may want to load your asset from here but you can load it before if you prefer.

   .. code::

      print '1'
      function drystal.init()
         print '3'
      end
      print '2'

.. lua:function:: update(dt: float)

   This functions is called at each frame. The ``dt`` parameter represents the time elapsed since last update (in seconds).
   Use this function to update the game.

.. lua:function:: draw()

   This function is called after update. 

.. lua:function:: atexit()

   This function is called when the window is closed or when :ref:`drystal.stop <drystal-stop>` is called.

.. lua:function:: prereload()

   Called before reload of the game (by :ref:`reload <drystal-reload>`, by pressing F3 or with livereloading).
   You can use it to save current state of the game inside a global so it is still accessible after the reload.

.. lua:function:: postreload()

   Called after reload of the game.
   You can use it to restore the state of the game.

.. include that in a tutorial
.. .. literalinclude:: ../examples/red_background.lua
..    :language: lua
..    :linenos:

Event
-----

.. .. lua:function:: set_relative_mode(relative: boolean)

.. _start-text:
.. lua:function:: start_text()

   Begin text mode. Instead of receiving key_press/key_release event, key_text will be received.
   In text mode, you get the unicode caracter corresponding to the key pressed (and modifiers).

.. lua:function:: stop_text()

   Stop the text mode.


Callbacks
^^^^^^^^^

To receive events, you have to defined some of the following functions.

.. lua:function:: mouse_motion(x, y, dx, dy)

   Called when the mouse is moved. ``dx`` and ``dy`` are difference betweend the current position and the last one.

.. _mouse-press:
.. lua:function:: mouse_press(x, y, button: int)

   Called when a button (or mouse wheel) is pressed.

   :param: button can be: 1 for left click, 2 for middle click, 3 for right click, 4 and 5 for mouse wheel.

.. lua:function:: mouse_release(x, y, button)

   Called when a button (or mouse wheel) is released.

   :param: button is the same as in :ref:`mouse_press <mouse-press>`.

.. _key-press:
.. lua:function:: key_press(key)

   Called when a key is pressed.
   Depending on key repeat system configuration of the player, ``key_press`` can be called multiple times even if the user didn't released the key. ``key_release`` will be called too.

.. lua:function:: key_release(key)

   Same as :ref:`key_press <key-press>` but when a key is released.

.. lua:function:: key_text(unicode_key)

   Called in text mode (see :ref:`start_text <start-text>`).

.. .. lua:function:: resize_event(w, h)

.. code::

   function drystal.mouse_motion(x, y)
      print('mouse is at', x, y)
   end


Graphics
--------

Window
^^^^^^

.. lua:data:: screen

   `screen` is the surface representing the window/canvas, which will be blit after execution of the `drystal.draw` callback.

.. lua:data:: current_draw_on

   :ref:`Surface:draw_on() <Surface-draw-on>`

.. lua:data:: current_draw_from

   :ref:`Surface:draw_from() <Surface-draw-from>`

.. lua:function:: resize(width: int, height: int)

   Resize the window (canvas in case of web build) to the dimensions specified.

   .. note:: Unlike some engines, you can resize the window without having to recreate your surfaces or shaders.

   .. code-block:: lua
      :linenos:

      drystal.resize(200, 300)
      assert(drystal.screen.w == 200)
      assert(drystal.screen.h == 300)

.. lua:function:: set_fullscreen(fullscreen: bool)

   Enable or disable the fullscreen mode.

   .. note:: It will make the game fits the whole page in a browser and not use
             the fullscreen mode. We choose this behavior because the echap key will not
             be available anymore and it makes less sense to play a game in fullscreen with a browser.
             You also need to ensure that there is no border or margin for the canvas in your index.html.
   .. code-block:: css

      html,body {
          margin: 0;
          padding: 0;
          width: 100%;
          height: 100%;
          overflow: hidden;
      }

.. lua:function:: set_title(title: str)

   Change the title of the window. In Web build, the title of the document is changed.

.. lua:function:: show_cursor(show: bool)

   Decide if the mouse cursor should be hidden or not.

Surface
^^^^^^^

.. lua:class:: Surface

    Object representing a surface. Surfaces can be drawn on other surfaces (screen included).

   .. lua:data:: w

      Width of the surface.

   .. lua:data:: h

      Height of the surface.

   .. _Surface-draw-on:
   .. lua:method:: draw_on() -> Surface

      Use this surface as destinatin/backbuffer (draw method be redirected to this surface instead of screen) for futur draws.

      :return: the old surface which was used

   .. _Surface-draw-from:
   .. lua:method:: draw_from() -> Surface

      Use this surface as source for futur textured draws (like ``drystal.draw_sprite``).

      :return: the old surface which was used

   .. lua:method:: set_filter(filter)

      :param: filter is one of ``drystal.NEAREST``, ``drystal.LINEAR``, ``drystal.BILINEAR`` or ``drystal.TRILINEAR``.


.. lua:function:: new_surface(width, height)

   Create new surface of dimensions (``width``, ``height``).
   By default, the surface is black.

   .. code::

      local surf = drystal.new_surface(200, 200)
      surf:draw_on() -- the following draw function will act on this surface
      drystal.set_color(255, 255, 255)
      drystal.draw_circle(surf.w / 2, surf.h / 2, 100) -- draw a white circle inside the surface
      drystal.screen:draw_on()
      ...


.. lua:function:: load_surface(filename)

   Load a surface from a file.
   If the file doesn't exist or is invalid, ``load_surface`` returns (`nil`, error).

   .. note:: Use :lua:`assert(drystal.load_surface 'test.png')` to make sure the surface is loaded.


Drawing primitives
^^^^^^^^^^^^^^^^^^

.. _set-color:
.. lua:function:: set_color(red: float [0-255], green: float [0-255], blue: float [0-255])

   Set current color, used by ``draw_*`` functions.

.. _set-alpha:
.. lua:function:: set_alpha(alpha: float [0-255])

   Set current alpha, used by ``draw_*`` functions.

.. lua:function:: set_line_width(width: float)

   Set current line width, used by :ref:`draw_line <draw_line>`.

.. lua:function:: set_point_size(size float)

   Set current point size, used by :ref:`draw_point <draw_point>`.

.. lua:function:: draw_background()

   Clear current `draw_on` surface.

.. note:: In the following function, ``x``, ``y``, ``w`` (width) and ``h`` (height) are floats. Angle are expressed in radians. ``x`` and ``y`` are screen coordinates.

.. _draw_point:
.. lua:function:: draw_point(x, y)

   Draw a point at the given coordinate.

.. .. lua:function:: draw_point_tex()

.. _draw_line:
.. lua:function:: draw_line(x1, y1, x2, y2)

   Draw a line between the two given points.

.. lua:function:: draw_triangle(x1, y1, x2, y2, x3, y3)

   Draw a filled triangle between the three given points.

.. _draw-surface:
.. lua:function:: draw_surface(ix1, iy1, ix2, iy2, ix3, iy3, ox1, oy1, ox2, oy2, ox3, oy3)

   Draw a surface (set as `current draw from`). The first 6 parameters represent a triangle in the source texture, the last 6 represent the destination triangle. The can have different size to create deformations.

   Tinting is possible by using :ref:`set_color <set-color>` (255, 255, 255 for no modification).

.. lua:function:: draw_quad(ix1, iy1, ix2, iy2, ix3, iy3, ix4, iy4, ox1, oy1, ox2, oy2, ox3, oy3, ox4, oy4)

   Same as :ref:`draw_surface <draw-surface>` but with quadrilaterals instead of triangles.

.. lua:function:: draw_rect(x, y, w, h)

   Draw a filled rectangle.

.. lua:function:: draw_rect_rotated(x, y, w, h, angle: float)

   Draw a filled rotated rectangle.

.. lua:function:: draw_square(x, y, w, h)

   Draw a non-filled rectangle.

.. lua:function:: draw_circle(x, y, radius: float)

   Draw a circle. The coordinate is the position of the center. ``radius`` is expressed in pixel.

    .. note:: Draw circle draws a lot of triangles. If possible, include a circle in your spritesheet and draw it with :ref:`draw_sprite <draw_sprite>`.

.. lua:function:: draw_polygon(x1, y1, x2, y2, ...)

   Draw a filled polygon.

.. lua:function:: draw_polyline(x1, y1, x2, y2, ...)

   Draw a non-filled polygon.

.. lua:function:: draw_image(x, y, w, h, destx, desty[, destw=w[, desth=h]])

   Draw an image. It can be resized if ``destw`` or ``desth`` are different than ``w`` and ``h``

.. _draw_sprite:
.. lua:function:: draw_sprite(sprite: table, x, y[, transform: table])

   Draw a sprite.
   Use :ref:`Sprite <sprite>` for easier sprite drawing.

   :param: sprite must have fields x, y, w and h
   :param: transform must have fields angle, wfactor and hfactor

   .. code::

      local sprite = { -- the first image of a 32x32 spritesheet
         x = 0,
         y = 0,
         w = 32,
         h = 32,
      }
      function drystal.draw()
         ...
         drystal.draw_sprite(sprite, 200, 300)
      end

.. lua:function:: draw_sprite_simple(sprite: table, x, y)
.. lua:function:: draw_sprite_rotated(sprite: table, x, y, angle: float)
.. lua:function:: draw_sprite_resized(sprite: table, x, y, w, h)


Blending
^^^^^^^^

.. todo:: Images to show the differences

.. lua:data:: BLEND_DEFAULT
.. lua:data:: BLEND_ALPHA
.. lua:data:: BLEND_ADD
.. lua:data:: BLEND_MULT

Camera
^^^^^^

.. lua:data:: x (=0)

   Position of the camera (x coordinate).

.. lua:data:: y (=0)

   Position of the camera (y coordinate).

.. lua:data:: zoom (=1)

   Zoom of the camera. Values greater than 1 mean zoom in, less than 1 mean zoom out.

.. lua:data:: angle (=0)

   Angle of the camera. You can easily apply a tilt effect with this field.

.. lua:function:: reset()

   Reset the camera fields to default values.


Buffer
^^^^^^

A buffer can **only contain one type of shape** (point, line, triangle, textured triangle).


.. lua:class:: Buffer

   .. lua:method:: use()

      Use this buffer as current buffer.

   .. lua:method:: draw([dx=0: float[, dy=0: float]])

      Draw this buffer. ``dx`` and ``dy`` can be used to offset the draw.

   .. lua:method:: reset()

      Remove all elements from the buffer.

   .. lua:method:: upload_and_free()

      Send the buffer to the graphic card and free memory.
      If a buffer is freed, you cannot call ``reset``, ``use`` or ``upload_and_free`` anymore or errors with be thrown.

.. lua:function:: new_buffer([size: int]) -> Buffer

   Create a buffer of the specified ``size``. ``size`` must be a multiple of the number of points of the shape you put in it.
   For example, if you put triangles, ``size`` must be a multiple of 3.

.. lua:function:: use_buffer()

   Use default drystal buffer.


Shader
^^^^^^

.. lua:class:: Shader

   .. lua:method:: use()

      Use a shader for the following draws.

   .. lua:method:: feed(uniform: str, value: float)

.. lua:function:: new_shader([vertex: str[, fragment_color: str[, fragment_texture: str]]]) -> Shader

   Create a shader with code specified.
   If one of the code is :lua:`nil`, code of the default shader is used.

.. lua:function:: use_shader()

   Use default drystal shader.

Post processing
"""""""""""""""

.. lua:function:: create_postfx(name: str, code: str[, uniforms: table]) -> function | (nil, error)

   Create a post processing effect.
   The ``code`` parameter should contains a *effect* function.
   Additionnal uniforms can be declared by the ``uniforms`` parameter.

   .. code::

      assert(drystal.create_postfx('gray', [[
         vec3 effect(sampler2D tex, vec2 coord)
         {
             vec3 texval = texture2D(tex, coord).rgb;
             return mix(texval, vec3((texval.r + texval.g + texval.b) / 3.0), scale);
         }
      ]], {'scale'}))

.. lua:function:: postfx(name: str, uniforms...: floats)

      Apply a post processing effect on the current *draw on* surface. The uniform list should have the same order than in the declaration of the effect.

   .. code::

      function drystal.draw()
         ...
         drystal.postfx('gray', 0.8)
      end


Sprite
""""""

.. _sprite:
.. lua:class:: Sprite

   .. lua:method:: draw()

      Draw the sprite on the current *draw on* surface.

.. lua:function:: new_sprite(table) -> Sprite

   Create a sprite.

   :param: ``table`` contains:

      - x, y coordinates,
      - optional color table,
      - optional alpha,
      - optional angle,
      - optional w, h (needed if source not defined),
      - optional source (a sprite table (x, y, w, h); if not defined, draws a rectangle),
      - any other fields you might need for your code.

.. code::

    local data = { x=0, y=0, w=32, h=32 }
    local sprite = drystal.new_sprite {
        x=350,
        y=200,
        w=sprite.w,
        h=sprite.h,
        source=sprite,              -- if nil, draw a rectangle
        update=function(self, dt)   -- custom field
            self.angle = self.angle + dt * math.pi * 2
        end
    })
    function drystal.update(dt)
        sprite:update(dt)
    end
    function drystal.draw()
        ... -- draw background, etc
        sprite:draw()
    end


Font rendering
--------------

.. lua:class:: Font

   .. _font-draw:
   .. lua:method:: draw(text: str, x, y[, alignment=1: int])

      Draw ``text`` at given coordinates.
      Supports '\\n'.
      A particular syntax can be used to create some text effects, for example:

         - :lua:`"test {r:255|g:0|b:0|!}"` will print the ``!`` in red,
         - :lua:`"{outline|outg:255|t{nooutline|e}st}"` will print ``test`` with a green outline, except the ``e``.

      :param:

         - if alignment is 1, text is left aligned (default)
         - if alignment is 2, text is centered around ``x``.
         - if alignment is 3, text is right aligned``x``.

   .. _font-draw-plain:
   .. lua:method:: draw_plain(text: str, x, y)

      Same as :ref:`draw <font-draw>`, except it doesn't align nor accept formating.
      Use this function for faster text drawing.

   .. lua:method:: sizeof(text) -> float, float

      Returns width and height the text would use if it was drawn on the screen.

   .. lua:method:: sizeof_plain(text)

      Returns width and height the text would use if it was drawn on the screen by :ref:`draw_plain <font-draw-plain>`.

.. lua:function:: load_font(filename: str, size: float) -> Font | (nil, error)

   Load a truetype font (.ttf file) at desired size.


Particle System
---------------

.. lua:class:: System

   .. lua:method:: start()
   .. lua:method:: pause()
   .. lua:method:: emit()
   .. lua:method:: stop()

   .. lua:method:: draw([x=0: float[, y=0: float]))
   .. lua:method:: update(dt: float)

   .. lua:method:: is_running() -> boolean
   .. lua:method:: set_running(run: boolean)

   .. lua:method:: add_size(at_lifetime, size)
   .. lua:method:: add_size(at_lifetime, minsize, maxsize)
   .. lua:method:: add_color(at_lifetime, r, g, b)
   .. lua:method:: add_color(at_lifetime, minr, maxr, ming, maxg, minb, maxg)

   .. lua:method:: set_position(x: float, y: float)
   .. lua:method:: get_position() -> float, float

   .. lua:method:: set_offset(x: float, y: float)
   .. lua:method:: get_offset() -> float, float

   .. lua:method:: set_lifetime(min: float[, max=min: float])
   .. lua:method:: get_lifetime() -> float, float
..    .. lua:method:: set_min_lifetime(min: float)
..    .. lua:method:: get_min_lifetime() -> float
..    .. lua:method:: set_max_lifetime(max: float)
..    .. lua:method:: get_max_lifetime() -> float

   .. lua:method:: set_direction(min: float[, max=min: float])
   .. lua:method:: get_direction() -> float, float
..    .. lua:method:: set_min_direction(min: float)
..    .. lua:method:: get_min_direction() -> float
..    .. lua:method:: set_max_direction(max: float)
..    .. lua:method:: get_max_direction() -> float

   .. lua:method:: set_initial_acceleration(min: float[, max=min: float])
   .. lua:method:: get_initial_acceleration() -> float, float
..    .. lua:method:: set_min_initial_acceleration(min: float)
..    .. lua:method:: get_min_initial_acceleration() -> float
..    .. lua:method:: set_max_initial_acceleration(max: float)
..    .. lua:method:: get_max_initial_acceleration() -> float

   .. lua:method:: set_initial_velocity(min: float[, max=min: float])
   .. lua:method:: get_initial_velocity() -> float, float
..    .. lua:method:: set_min_initial_velocity(min: float)
..    .. lua:method:: get_min_initial_velocity() -> float
..    .. lua:method:: set_max_initial_velocity(max: float)
..    .. lua:method:: get_max_initial_velocity() -> float

.. lua:function:: new_system() -> System

Physic
------

Learning how to use Box2D_ will help to hunder Drystal's physic module.

.. lua:function:: create_world(gravity_x: float, gravity_y: float)

   .. warning:: If ``create_world`` isn't called before other physic functions, errors will occur.

.. lua:function:: update_physic(dt: float)

   Update the world.

.. lua:function:: on_collision(todo)
.. lua:function:: raycast(todo)

.. lua:function:: query(x1, y1, x2, y2) -> table

   Return a table with all bodies contains inside the area defined by ``x1``, ``y1``, ``x2`` and ``y1``.

.. lua:function:: new_shape('box', width, height, centerx, centery)
.. lua:function:: new_shape('circle', radius)
.. lua:function:: new_shape('chain', x1, y1, x2, y2, ...)

.. lua:function:: new_body(is_dynamic: boolean, [x, y], shape1, shape2, ...)

   Create a body at given position (or 0, 0) with given shapes. If ``is_dynamic`` is false, the body will be static.

.. lua:function:: new_joint(todo)

.. lua:class:: Shape

   .. lua:method:: set_density(density: float)
   .. lua:method:: get_density() -> float
   .. lua:method:: set_restitution(restitution float)
   .. lua:method:: get_restitution() -> float
   .. lua:method:: set_friction(friction float)
   .. lua:method:: get_friction() -> float
   .. lua:method:: set_sensor(sensor: boolean)

.. lua:class:: Body

   .. lua:method:: set_position(x: float, y: float)
   .. lua:method:: get_position() -> float, float
   .. lua:method:: set_angle(angle: float)
   .. lua:method:: get_angle() -> float
   .. lua:method:: set_linear_velocity(x: float, y: float)
   .. lua:method:: get_linear_velocity() -> float, float
   .. lua:method:: set_angular_velocity(x: float, y: float)
   .. lua:method:: get_angular_velocity() -> float, float
   .. lua:method:: set_linear_damping(damping: float)
   .. lua:method:: get_linear_damping() -> float
   .. lua:method:: set_angular_damping(damping: float)
   .. lua:method:: get_angular_damping() -> float
   .. lua:method:: set_fixed_rotation(fixed: boolean)
   .. lua:method:: get_fixed_rotation() -> boolean

   .. lua:method:: set_active(active: boolean)
   .. lua:method:: set_bullet(bullet: boolean)
   .. lua:method:: get_mass() -> float
   .. lua:method:: set_mass_center(x, y)
   .. lua:method:: apply_force(x, y)
   .. lua:method:: apply_linear_impulse(x, y)
   .. lua:method:: apply_angular_impulse(angle)
   .. lua:method:: apply_torque(torque)
   .. lua:method:: dump()
   .. lua:method:: destroy()

.. lua:class:: Joint

   .. lua:method:: set_target(x, y)

      .. warning:: only for mouse joint

   .. lua:method:: set_length(length: float)

      .. warning:: only for distance joint

   .. lua:method:: set_frequency(frequency: float)

      .. warning:: only for distance joint

   .. lua:method:: set_max_length(max_length: float)

      .. warning:: only for distance joint

   .. lua:method:: set_angle_limits(min, max)

      .. warning:: only for revolute joint

   .. lua:method:: set_motor_speed(speed: float[, maxtorque=20: float])

      .. warning:: only for revolute joint

   .. lua:method:: destroy()

Audio
-----

.. lua:class:: Music

   .. lua:method:: play()

      Play the music (from the beginning).

   .. lua:method:: stop()

      Stop the music.

.. lua:function:: load_music(filename: str) -> Music | (nil, error)

   Load a music from a file. It has to be in Ogg_ format.

.. lua:function:: load_music(callback: function[, samplesrate=44100: int]) -> Music | (nil, error)
.. lua:function:: set_music_volume(volume: float [0-1])

.. lua:class:: Sound

   .. lua:method:: play([volume=1[, x=0[, y=0]]])

      Play the sound at given volume and position.

      :param: volume float between 0 and 1
      :param: x float between -1 and 1 (-1 is full left, 1 is full right)
      :param: y float between -1 and 1

.. lua:function:: load_sound(filename: str) -> Sound | (nil, error)

   Load a sound from a file. It has to be in WAV_ format. Only 44100Hz, 8 bits or 16 bits are supported.
   If you want to use positional audio, it has to be mono audio.

.. lua:function:: load_sound(callback: function, numsamples: int) -> Sound | (nil, error)
.. lua:function:: load_sound(data: table) -> Sound | (nil, error)
.. lua:function:: set_sound_volume(volume: float [0-1])

Storage
-------

In browser, this module uses Javascript's localStorage feature. In desktop, it uses files.

Storage is a way to store data for futur executions.

.. lua:function:: store(key: str, value: table)

   Store a table in the storage.

.. lua:function:: fetch(key: str) -> table | nil

   Retrieve the table associated with the given key.

.. code::

   drystal.store('test', {text='wow'})
   assert(drystal.fetch('test').text == 'wow')

.. note:: Serialization of the table is done by a JSON module.
      In web, data are available in ``localStorage`` from Javascript.


Web
---

.. lua:data:: is_web: boolean

   Equals ``true`` if the game is executed inside a browser.

.. lua:function:: wget(url: string, filename: string, onload: function, onerror: function)

   .. warning:: ``wget`` throws an error in native build.

.. lua:function:: run_js(script: str)

   .. warning:: ``run_js`` throws an error in native build.


Utils
-----

.. lua:function:: display_logo(sprite: {x, y, w, h}, background: {r, g, b})

.. lua:function:: file_exists(filename: str) -> boolean

   Returns ``true`` if the file exists.

.. _Ogg: https://en.wikipedia.org/wiki/Ogg
.. _WAV: https://en.wikipedia.org/wiki/WAV
.. _Box2D: http://box2d.org/
