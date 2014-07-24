.. highlightlang:: lua
   :linenothreshold: 3

.. lua:module:: drystal

.. role:: lua(code)
   :language: lua

API Reference
=============

Engine
------

.. lua:function:: stop()

    Stop execution of drystal.

.. lua:function:: reload()

    Reload the current game. This can be useful during the development in native build.

    .. note:: F3 also reloads the game.


Event
-----

.. .. lua:function:: set_relative_mode(relative: boolean)

.. lua:function:: start_text()

.. lua:function:: stop_text()


Callbacks
^^^^^^^^^

.. lua:function:: mouse_motion(x, y, dx, dy)
.. lua:function:: mouse_press(x, y, button)
.. lua:function:: mouse_release(x, y, button)
.. lua:function:: key_press(key)
.. lua:function:: key_release(key)
.. lua:function:: key_text(unicode_key)
.. .. lua:function:: resize_event(w, h)


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

.. lua:function:: set_title(title: str)

    Change the title of the window. In Web build, this has no effect.

    .. todo:: Maybe ``set_title`` should modify the title of the HTML page.

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

        Use this surface as backbuffer (draw method be redirected to this surface instead of screen).

        :return: the old surface which was used

    .. _Surface-draw-from:
    .. lua:method:: draw_from() -> Surface

        Use this surface for textured draws (like ``drystal.draw_sprite``).

        :return: the old surface which was used

    .. lua:method:: set_filter(filter)

        :param: filter is one of ``drystal.NEAREST``, ``drystal.LINEAR``, ``drystal.BILINEAR`` or ``drystal.TRILINEAR``.


.. lua:function:: new_surface(width, height)

    Create new surface of dimensions (``width``, ``height``).
    By default, the surface is black.

    .. code:: lua

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

.. lua:function:: set_color(red: float [0-255], green: float [0-255], blue: float [0-255])

    Set current color, used by ``draw_*`` functions.

.. lua:function:: set_alpha(alpha: float [0-255])

    Set current alpha, used by ``draw_*`` functions.

.. lua:function:: set_line_width(width: float)

    Set current line width, used by :ref:`draw_line <draw_line>`.

.. lua:function:: set_point_size(size float)

    Set current point size, used by :ref:`draw_point <draw_point>`.

.. lua:function:: draw_background()

    Clear current `draw_on` surface.

.. note:: In the following function, ``x``, ``y``, ``w`` (width) and ``h`` (height) are floats. Angle are expressed in radian.

.. _draw_point:
.. lua:function:: draw_point(x, y)

.. .. lua:function:: draw_point_tex(x, y)

.. _draw_line:
.. lua:function:: draw_line(x1, y1, x2, y2)

.. lua:function:: draw_triangle(x1, y1, x2, y2, x3, y3)

.. lua:function:: draw_surface(ix1, iy1, ix2, iy2, ix3, iy3, ox1, oy1, ox2, oy2, ox3, oy3)

.. lua:function:: draw_quad(ix1, iy1, ix2, iy2, ix3, iy3, ix4, iy4, ox1, oy1, ox2, oy2, ox3, oy3, ox4, oy4)

.. lua:function:: draw_rect(x, y, w, h)
.. lua:function:: draw_rect_rotated(x, y, w, h, angle: float)
.. lua:function:: draw_square(x, y, w, h)

.. lua:function:: draw_circle(x, y, radius)

    .. note:: Draw circle draws a lot of triangles. If possible, include a circle in your spritesheet and draw it with :ref:`draw_sprite <draw_sprite>`

.. lua:function:: draw_polygon(x1, y1, x2, y2, ...)
.. lua:function:: draw_polyline(x1, y1, x2, y2, ...)

.. lua:function:: draw_image(x, y, w, h, destx, desty[, destw=w[, dh=h]])

.. _draw_sprite:
.. lua:function:: draw_sprite(sprite: table, x, y[, transform: table])
.. lua:function:: draw_sprite_simple(sprite: table, x, y)
.. lua:function:: draw_sprite_rotated(sprite: table, x, y, angle: float)
.. lua:function:: draw_sprite_rotated(sprite: table, x, y, w, h)


Blending
^^^^^^^^

.. todo:: Images to show the differences

.. lua:data:: BLEND_DEFAULT
.. lua:data:: BLEND_ALPHA
.. lua:data:: BLEND_ADD
.. lua:data:: BLEND_MULT

Camera
^^^^^^

.. lua:data:: x
.. lua:data:: y
.. lua:data:: zoom
.. lua:data:: angle

.. lua:function:: reset()


Buffer
^^^^^^

.. lua:class:: Buffer

    .. lua:method:: use()

    .. lua:method:: draw([dx=0: float[, dy=0: float]])

    .. lua:method:: reset()

    .. lua:method:: upload_and_free()

.. lua:function:: new_buffer([size: int]) -> Buffer

.. lua:function:: use_buffer()

Shader
^^^^^^

.. lua:class:: Shader

    .. lua:method:: use()

    .. lua:method:: feed(uniform: str, value: float)

.. lua:function:: new_shader([vertex: str[, fragment_color: str[, fragment_texture: str]]]) -> Shader

.. lua:function:: use_shader()

Post processing
"""""""""""""""

.. lua:function:: create_postfx(name: str, code: str[, unforms: table]) -> function | (nil, error)

    .. code:: lua

        assert(drystal.create_postfx('gray', [[
            vec3 effect(sampler2D tex, vec2 coord)
            {
                vec3 texval = texture2D(tex, coord).rgb;
                return mix(texval, vec3((texval.r + texval.g + texval.b) / 3.0), scale);
            }
        ]], {'scale'}))

.. lua:function:: postfx(name: str, unforms...: floats)

    .. code:: lua

        function drystal.draw()
            ...
            drystal.postfx('gray', 0.8)
        end

Sprite
""""""

.. lua:class:: Sprite

    .. lua:method:: draw()

.. lua:function:: new_sprite(table)

.. code:: lua

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

    .. lua:method:: draw(text: str, x, y)
    .. lua:method:: draw_plain(text: str, x, y)
    .. lua:method:: draw_align(text: str, x, y, alignemt: str)
    .. lua:method:: draw_align_plain(text: str, x, y, alignemt: str)

    .. lua:method:: sizeof(text)
    .. lua:method:: sizeof_plain(text)

.. lua:function:: load_font(filename: str, size: float) -> Font | (nil, error)


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

.. lua:function:: create_world(gravity_x: float, gravity_y: float)

    .. warning:: If ``create_world`` isn't called before other physic functions, errors will occur.

.. lua:function:: update_physic(dt: float)

.. lua:function:: on_collision(todo)
.. lua:function:: raycast(todo)
.. lua:function:: query(todo)

.. lua:function:: new_shape(todo)
.. lua:function:: new_body(todo)
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
    .. lua:method:: stop()

.. lua:function:: load_music(filename: str) -> Music | (nil, error)
.. lua:function:: load_music(callback: function[, samplesrate=44100: int]) -> Music | (nil, error)
.. lua:function:: set_music_volume(volume: float [0-1])

.. lua:class:: Sound

    .. lua:method:: play([volume=1[, x=0[, y=0]]])

.. lua:function:: load_sound(filename: str) -> Sound | (nil, error)
.. lua:function:: load_sound(callback: function) -> Sound | (nil, error)
.. lua:function:: load_sound(data: table) -> Sound | (nil, error)
.. lua:function:: set_sound_volume(volume: float [0-1])

Storage
-------

.. lua:function:: store(key: str, value: table)
.. lua:function:: fetch(key: str) -> table

.. code:: lua

    drystal.store('test', {text='wow'})
    assert(drystal.fetch('test').text == 'wow')

.. note:: Serialization of the table is done by a JSON module.
        In web, data are available in ``localStorage`` from Javascript.


Web
---

.. lua:function:: is_web() -> boolean

    Returns ``true`` if the game is executed inside a browser.

.. lua:function:: wget(url: string, filename: string, onload: function, onerror: function)

    .. warning:: ``wget`` doesn't work in native build.

.. lua:function:: run_js(script: str)

    .. warning:: ``run_js`` doesn't work in native build.


Utils
-----

.. lua:function:: display_logo(sprite: {x, y, w, h}, background: {r, g, b})

.. lua:function:: file_exists(filename: str) -> boolean

    Returns ``true`` if the file exists.

