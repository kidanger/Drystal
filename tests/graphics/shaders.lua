local drystal = require 'drystal'

local shader

local vert = [[
#ifdef GL_ES
precision highp float;
#endif

attribute vec2 position;
attribute vec4 color;
attribute vec2 texCoord;
attribute float pointSize;

varying vec4 fColor;
varying vec2 fTexCoord;

uniform vec2 destinationSize;

uniform float mx;
uniform float my;

void main()
{
	gl_PointSize = pointSize;
	vec2 pos = 2. * position / destinationSize - 1.;
	gl_Position = vec4(pos, mx*my+pos.x*pos.y, 1.0);
	vec3 offcolor = vec3(pos, pos.x + pos.y) - vec3(mx, my, mx*my);
	fColor = color - vec4(offcolor, 0.);
	fTexCoord = texCoord;
}
]]
local fragcolor = [[
]]
local fragtex = [[
]]

local mx, my
local width = 1024
local height = 768

function drystal.init()
	drystal.resize(width, height)
	shader = assert(drystal.new_shader(vert, fragcolor, fragtex))
end

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	shader:use()
	drystal.set_color(255, 255, 255)
	drystal.draw_rect(0, 0, width, height)
	for ii = 0, 5 do
		local i = ii / 10
		drystal.draw_rect(width*i, height*i, width*(1-i*2), height*(1-i*2))
	end

	drystal.use_default_shader()
end

function drystal.key_press(key)
	if key == 'a' then
		drystal.stop()
	end
end

function drystal.mouse_motion(x, y)
	mx, my = x, y
	shader:feed('mx', (mx/width) * 2 - 1)
	shader:feed('my', (1-my/height) * 2 - 1)
end

