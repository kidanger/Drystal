local drystal = require 'drystal'

local weird_shader_vertex = [[
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
	pos.x += rand(vec2(p1.x, tick/10000.+100.)) * .05;
	pos.y += rand(vec2(p1.y, tick/10000.)) * .05;

	gl_Position = vec4(pos, 0., 1.);
	fColor = color;
	fTexCoord = texCoord;
}
]]

function drystal.display_logo(sprite, background)
	background = background or {255, 255, 255}
	local duration = 6
	local shader = assert(drystal.new_shader(weird_shader_vertex))
	if shader then
		shader:use()
	end
	local time = 0
	local _update = drystal.update
	local _draw = drystal.draw
	local _key_press = drystal.key_press

	local wr, hr = drystal.screen.w, drystal.screen.h
	local ws = math.min(wr, sprite.w) - 50
	local hs = math.min(hr, sprite.h) - 50

	drystal.update = function(dt)
		time = time + dt
		if time > duration then
			drystal.update = _update
			drystal.draw = _draw
			drystal.key_press = _key_press
			drystal.set_alpha(255)
		end
	end
	local transform = {
		angle=0,
		wfactor=ws/sprite.w,
		hfactor=hs/sprite.h,
	}
	drystal.draw = function()
		if shader then shader:feed('tick', time*100) end

		drystal.set_alpha(255)
		drystal.set_color(unpack(background))
		drystal.draw_background()

		drystal.set_color(255, 255, 255)
		local a = 255 - (math.cos(time*math.pi/(duration/2))/2+0.5)*255
		drystal.set_alpha(a)
		drystal.draw_sprite(sprite, (wr-ws)/2, (hr-hs)/2, transform)
	end
	drystal.key_press = function()
		time = duration
	end
end

function drystal.file_exists(name)
	local f = io.open(name, "r")
	if f then
		f:close(f)
		return true
	end
	return false
end

