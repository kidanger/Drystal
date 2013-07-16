require "data/drystal"

local json = require 'data/dkjson'
spritesheet = json.decode(io.open('data/image.json'):read('*all'))

vert = [[
#version 100
#ifdef GL_ES
precision highp float;
#endif

attribute vec2 position;
attribute vec4 color;
attribute vec2 texCoord;

varying vec4 fColor;
varying vec2 fTexCoord;

#define pi ]]..math.pi..[[

uniform float tick;

float rand(vec2 co){
	return sin(dot(co.xy, vec2(12.9898,78.233)));
}

void main()
{
	vec2 p1 = position;
	vec2 pos = position.xy;
	pos.x += rand(vec2(p1.x, tick/10000.+100.)) * .01;
	pos.y += rand(vec2(p1.y, tick/10000.)) * .01;

	gl_Position = vec4(pos, 0., 1.);
	fColor = color;
	fTexCoord = texCoord;
}
]]

x = 0
y = 0
width = 0
height = 0
function init()
	print("initialized from lua")
	resize(600, 400)
	set_resizable(false)
	image = load_surface('data/' .. spritesheet.meta.image)
	draw_from(image)
	surf = new_surface(64, 32)
	draw_on(surf)
	set_color(255, 0, 0)
	set_alpha(105)
	draw_rect(32, 0, 40, 40)

	set_color(200, 200, 200)
	set_alpha(255)
	local sprite = spritesheet.frames['character.png'].frame
	draw_sprite(sprite, 0, 0)
	draw_on(screen)

	shader = new_shader(vert)
	if not shader then
		engine_stop()
	end
	display_logo(spritesheet.frames['logo.png'].frame)
end

tick = 0
function update(dt)
	tick = tick + dt
end

function draw()
	draw_from(image)
	set_alpha(255)
	set_color(10, 10, 30)
	draw_background()

	use_shader(shader)
	feed_shader(shader, 'tick', tick)
	set_color(255, 0, 0)
	set_alpha(105)
	draw_rect(16, 64, 40, 40)

	local sprite = spritesheet.frames['character.png'].frame
	set_color(200, 200, 200)
	set_alpha(255)
	draw_sprite_rotated(sprite, 16, 16, math.sin(tick/100))
	use_shader()

	set_color(100, 0, 0)
	set_alpha(200)
	draw_sprite(sprite, 16+32, 16)

	set_color(255,255,255)
	set_alpha((math.sin(tick/500)/2+0.5)*255)
	draw_from(surf)

	draw_image(0, 0, 64, 32, 0, 256)
	draw_from(image)

	flip()
end

function key_press(key)
	if key == 'escape' then
		engine_stop()
	end
end

function key_release(key)
end

function mouse_motion(_x, _y)
	x = _x
	y = _y
end

function _resize_event(w, h)
	width = w
	height = h
	resize(w, h)
	use_shader(shader)
end

