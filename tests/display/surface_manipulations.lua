local drystal = require "drystal"

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

vert = [[
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

local x = 0
local y = 0
local width = 0
local height = 0
function drystal.init()
	print("initialized from lua")
	drystal.resize(600, 400)
	image = drystal.load_surface(spritesheet.meta.image)
	drystal.draw_from(image)
	drystal.set_alpha(0)
	surf = drystal.new_surface(64, 32)
	drystal.draw_on(surf)
	drystal.set_color(255, 0, 0)
	drystal.set_alpha(105)
	drystal.draw_rect(32, 0, 40, 40)

	drystal.set_color(200, 200, 200)
	drystal.set_alpha(255)
	local sprite = spritesheet.frames['character.png'].frame
	drystal.draw_sprite(sprite, 0, 0)
	drystal.draw_on(drystal.screen)

	shader = drystal.new_shader(vert)
	if not shader then
		drystal.engine_stop()
	end
	drystal.display_logo(spritesheet.frames['logo.png'].frame)
end

local tick = 0
function drystal.update(dt)
	tick = tick + dt
end

function drystal.draw()
	drystal.draw_from(image)
	drystal.set_alpha(255)
	drystal.set_color(10, 10, 30)
	drystal.draw_background()

	drystal.use_shader(shader)
	drystal.feed_shader(shader, 'tick', tick)
	drystal.set_color(255, 0, 0)
	drystal.set_alpha(105)
	drystal.draw_rect(16, 64, 40, 40)

	local sprite = spritesheet.frames['character.png'].frame
	drystal.set_color(200, 200, 200)
	drystal.set_alpha(255)
	drystal.draw_sprite_rotated(sprite, 16, 16, math.sin(tick/100))
	drystal.use_shader()

	drystal.set_color(100, 0, 0)
	drystal.set_alpha(200)
	drystal.draw_sprite(sprite, 16+32, 16)

	drystal.set_color(255,255,255)
	drystal.set_alpha((math.sin(tick/500)/2+0.5)*255)
	drystal.draw_from(surf)

	drystal.draw_image(0, 0, 64, 32, 0, 256)
	drystal.draw_from(image)

	drystal.flip()
end

function drystal.key_press(key)
	if key == 'escape' then
		drystal.engine_stop()
	end
end

function drystal.key_release(key)
end

function drystal.mouse_motion(_x, _y)
	x = _x
	y = _y
end

function _resize_event(w, h)
	width = w
	height = h
	drystal.resize(w, h)
	drystal.use_shader(shader)
end

