local _draw_from = draw_from
local _draw_on = draw_on
local _draw_surface = draw_surface
draw_freeshape = _draw_surface

function _draw_rect(x, y, w, h)
	draw_triangle(x, y, x, y+h, x+w, y)
	draw_triangle(x+w, y, x+w, y+h, x, y+h)
end

function draw_image(x, y, w, h, dx, dy, dw, dh)
	dw = dw or w
	dh = dh or h
	_draw_surface(x, y,  x+w,   y,  x+w,   y+h,   x,  y+h,
				dx, dy, dx+dw, dy, dx+dw, dy+dh, dx, dy+dh)
end

local offsets = {{x=0,y=0}}
local ox = 0
local oy = 0

function get_offset()
	return ox, oy
end

local notransform = {
	angle=0, -- in radians
	wfactor=1, hfactor=1 -- can be negative to flip
}
function draw_sprite_simple(sprite, x, y)
	x = x + ox
	y = y + oy
	local w = sprite.w
	local h = sprite.h
	local x1, y1 = x, y
	local x2, y2 = x+w, y+h

	local xi = sprite.x
	local yi = sprite.y
	local xi2 = sprite.x + sprite.w
	local yi2 = sprite.y + sprite.h

	_draw_surface(xi, yi, xi2, yi, xi2, yi2, xi, yi2,
				  x1, y1, x2,  y1, x2,  y2,  x1, y2)
end
function draw_sprite(sprite, x, y, transform)
	if not transform then
		draw_sprite_simple(sprite, x, y)
		return
	end

	local w = sprite.w * transform.wfactor
	local h = sprite.h * transform.hfactor
	local angle = transform.angle
	local cos = math.cos(angle)
	local sin = math.sin(angle)

	x = x + ox
	y = y + oy

	function rot(_x, _y)
		return x + _x*cos - _y*sin + w/2,
				y + _y*cos + _x*sin + h/2
	end
	local x1, y1 = rot(-w/2, -h/2)
	local x2, y2 = rot(w/2, -h/2)
	local x3, y3 = rot(w/2, h/2)
	local x4, y4 = rot(-w/2, h/2)

	local xi = sprite.x
	local yi = sprite.y
	local xi2 = sprite.x + sprite.w
	local yi2 = sprite.y + sprite.h

	_draw_surface(xi, yi, xi2, yi, xi2, yi2, xi, yi2,
				  x1, y1, x2,  y2, x3,  y3,  x4, y4)
end
function draw_sprite_rotated(sprite, x, y, angle)
	local transform = {
		angle=angle,
		wfactor=1, hfactor=1
	}
	draw_sprite(sprite, x, y, transform)
end

local current_from = nil
function draw_from(surf)
	_draw_from(surf)
	current_from = surf
end
local current_on = nil
function draw_on(surf)
	_draw_on(surf)
	current_on = surf
end

function draw_surface(surf, x, y)
	local old = current_from

	draw_from(surf)
	local w, h = surface_size(surf)
	draw_image(0, 0, w, h, x+ox, y+oy)

	if old then
		draw_from(old)
	end
end

function draw_circle(cx, cy, r)
	cx = cx + ox;
	cy = cy + oy;
	
	
	local width, height = surface_size(current_on or screen)
	if cx + r < 0 or cx - r > width then
		return
	end
	if cy + r < 0 or cy - r > height then
		return
	end
	-- http://slabode.exofire.net/circle_draw.shtml

	local num_segments = math.floor(10*math.sqrt(r))
	local theta = 2 * 3.1415926 / num_segments
	local c = math.cos(theta) -- precalculate the sine and cosine
	local s = math.sin(theta)
	local t

	local x = r -- we start at angle = 0
	local y = 0

	local oldx, oldy
	local nx, ny
	for ii = 0, num_segments do
		oldx = nx
		oldy = ny

		-- apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;

		nx = x + cx
		ny = y + cy

		if ii > 0 then
			draw_triangle(cx, cy, nx, ny, oldx, oldy)
		end
	end
end

function draw_rect(x, y, w, h)
	_draw_rect(x+ox, y+oy, w, h)
end
function draw_square(x, y, w, h)
	x = x + ox
	y = y + oy
	draw_line(x, y, x+w, y)
	draw_line(x, y+h, x+w, y+h)
	draw_line(x, y, x, y+h)
	draw_line(x+w, y, x+w, y+h)
end

function draw_rect_rotated(x, y, w, h, angle)
	x = x + ox
	y = y + oy
	local cos = math.cos(angle)
	local sin = math.sin(angle)
	function rot(_x, _y)
		return x + _x*cos - _y*sin + w/2,
				y + _y*cos + _x*sin + h/2
	end
	local x1, y1 = rot(-w/2, -h/2)
	local x2, y2 = rot(w/2, -h/2)
	local x3, y3 = rot(w/2, h/2)
	local x4, y4 = rot(-w/2, h/2)
	draw_triangle(x1, y1, x2, y2, x3, y3)
	draw_triangle(x1, y1, x4, y4, x3, y3)
end


local _load_surface = load_surface
function load_surface(filename)
	local surf = _load_surface(filename)
	if not surf then
		print('File', filename, 'not found')
	end
	return surf
end

function push_offset(_ox, _oy)
	table.insert(offsets, {x=_ox, y=_oy})
	ox = offsets[#offsets].x
	oy = offsets[#offsets].y
end
function pop_offset()
	table.remove(offsets, #offsets)
	ox = offsets[#offsets].x
	oy = offsets[#offsets].y
end

local _set_color = set_color
function set_color(r, g, b)
	if g ~= nil then
		_set_color(r, g, b)
	else
		_set_color(unpack(r))
	end
end

function rotate_surface(surf, a)
end
function resize_surface(surf, w, h)
end

local weird_shader_vertex = [[
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
	pos.x += rand(vec2(p1.x, tick/10000.+100.)) * .05;
	pos.y += rand(vec2(p1.y, tick/10000.)) * .05;

	gl_Position = vec4(pos, 0., 1.);
	fColor = color;
	fTexCoord = texCoord;
}
]]

function display_logo(sprite, background)
	background = background or {255, 255, 255}
	local duration = 6000
	local shader = new_shader(weird_shader_vertex)
	if shader then
		use_shader(shader)
	end
	local time = 0
	local _update = update
	local _draw = draw

	local wr, hr = surface_size(screen)
	local ws = math.min(wr, sprite.w) - 50
	local hs = math.min(hr, sprite.h) - 50

	update = function(dt)
		time = time + dt
		if time > duration then
			update = _update
			draw = _draw
			if shader then
				free_shader(shader)
			end
			set_alpha(255)
		end
	end
	local transform = {
		angle=0,
		wfactor=ws/sprite.w,
		hfactor=hs/sprite.h,
	}
	draw = function()
		if shader then feed_shader(shader, 'tick', time*0.1) end

		set_alpha(255)
		set_color(unpack(background))
		draw_background()

		set_color(255, 255, 255)
		local a = 255 - (math.cos(time*math.pi/(duration/2))/2+0.5)*255
		set_alpha(a)
		draw_sprite(sprite, (wr-ws)/2, (hr-hs)/2, transform)

		flip()
	end
end

function file_exists(name)
	local f=io.open(name,"r")
	if f~=nil then io.close(f) return true else return false end
end

