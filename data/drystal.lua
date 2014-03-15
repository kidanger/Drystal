local drystal = drystal

local _draw_from = drystal.draw_from
local _draw_on = drystal.draw_on
local _draw_quad = drystal.draw_quad
drystal.draw_freeshape = _draw_quad

function drystal.draw_rect(x, y, w, h)
	drystal.draw_triangle(x, y, x, y+h, x+w, y)
	drystal.draw_triangle(x+w, y, x+w, y+h, x, y+h)
end

function drystal.draw_image(x, y, w, h, dx, dy, dw, dh)
	dw = dw or w
	dh = dh or h
	_draw_quad(x, y,  x+w,   y,  x+w,   y+h,   x,  y+h,
				dx, dy, dx+dw, dy, dx+dw, dy+dh, dx, dy+dh)
end

local notransform = {
	angle=0, -- in radians
	wfactor=1, hfactor=1 -- can be negative to flip
}
function drystal.draw_sprite_simple(sprite, x, y)
	local w = sprite.w
	local h = sprite.h
	local x1, y1 = x, y
	local x2, y2 = x+w, y+h

	local xi = sprite.x
	local yi = sprite.y
	local xi2 = sprite.x + sprite.w
	local yi2 = sprite.y + sprite.h

	_draw_quad(xi, yi, xi2, yi, xi2, yi2, xi, yi2,
				  x1, y1, x2,  y1, x2,  y2,  x1, y2)
end
function drystal.draw_sprite(sprite, x, y, transform)
	if not transform then
		drystal.draw_sprite_simple(sprite, x, y)
		return
	end

	local w = sprite.w * math.abs(transform.wfactor)
	local h = sprite.h * math.abs(transform.hfactor)
	local angle = transform.angle
	local cos = math.cos(angle)
	local sin = math.sin(angle)

	local function rot(_x, _y)
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

	if transform.wfactor < 0 then
		xi, xi2 = xi2, xi
	end
	if transform.hfactor < 0 then
		yi, yi2 = yi2, yi
	end

	_draw_quad(xi, yi, xi2, yi, xi2, yi2, xi, yi2,
				  x1, y1, x2,  y2, x3,  y3,  x4, y4)
end
function drystal.draw_sprite_rotated(sprite, x, y, angle)
	local transform = {
		angle=angle,
		wfactor=1, hfactor=1
	}
	drystal.draw_sprite(sprite, x, y, transform)
end
function drystal.draw_sprite_resized(sprite, x, y, w, h)
	drystal.draw_image(sprite.x, sprite.y, sprite.w, sprite.h, x, y, w, h)
end


function drystal.draw_circle(cx, cy, r)
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
			drystal.draw_triangle(cx, cy, nx, ny, oldx, oldy)
		end
	end
end

function drystal.draw_polygon(...)
	local coords = {...}
	local cx, cy = 0, 0
	for i = 1, #coords, 2 do
		cx = cx + coords[i]
		cy = cy + coords[i + 1]
	end
	cx = cx / (#coords / 2)
	cy = cy / (#coords / 2)
	for i = 1, #coords - 2, 2 do
		drystal.draw_triangle(coords[i], coords[i+1], coords[i+2], coords[i+3], cx, cy)
	end
	drystal.draw_triangle(coords[#coords - 1], coords[#coords], coords[1], coords[2], cx, cy)
end

function drystal.draw_polyline(loop, ...)
	local coords = {...}
	for i = 1, #coords - 2, 2 do
		drystal.draw_line(coords[i], coords[i+1], coords[i+2], coords[i+3])
	end
	if loop then
		drystal.draw_line(coords[1], coords[2], coords[#coords-1], coords[#coords])
	end
end

function drystal.draw_square(x, y, w, h)
	drystal.draw_line(x, y, x+w, y)
	drystal.draw_line(x, y+h, x+w, y+h)
	drystal.draw_line(x, y, x, y+h)
	drystal.draw_line(x+w, y, x+w, y+h)
end

function drystal.draw_rect_rotated(x, y, w, h, angle)
	local cos = math.cos(angle)
	local sin = math.sin(angle)
	local function rot(_x, _y)
		return x + _x*cos - _y*sin + w/2,
				y + _y*cos + _x*sin + h/2
	end
	local x1, y1 = rot(-w/2, -h/2)
	local x2, y2 = rot(w/2, -h/2)
	local x3, y3 = rot(w/2, h/2)
	local x4, y4 = rot(-w/2, h/2)
	drystal.draw_triangle(x1, y1, x2, y2, x3, y3)
	drystal.draw_triangle(x1, y1, x4, y4, x3, y3)
end


local Sprite = {
	x=0,
	y=0,
	angle=0,
	w=0,
	h=0,
	source=nil,
	color={255, 255, 255},
	alpha=255,
}
local sprite_is_update_with = {
	x=true,
	y=true,
	w=true,
	h=true,
	angle=true,
	source=true,
}
Sprite.__index = function(self, key)
	return rawget(self.data, key) or rawget(self, key) or rawget(Sprite, key)
end
Sprite.__newindex = function(self, key, value)
	if sprite_is_update_with[key] then
		rawset(self.data, key, value)
		self._updated = true
	else
		rawset(self, key, value)
	end
end

drystal.__Sprite = Sprite

function drystal.Sprite(args)
	local sprite = setmetatable({data=args}, Sprite)
	sprite._updated = true
	return sprite
end

function Sprite:_update()
	local cos = math.cos(self.angle)
	local sin = math.sin(self.angle)
	local x = self.x
	local y = self.y
	local w = self.w
	local h = self.h
	local function rot(_x, _y)
		return x + _x*cos - _y*sin + w/2,
				y + _y*cos + _x*sin + h/2
	end
	self._x1, self._y1 = rot(-w/2, -h/2)
	self._x2, self._y2 = rot(w/2, -h/2)
	self._x3, self._y3 = rot(w/2, h/2)
	self._x4, self._y4 = rot(-w/2, h/2)
	self._updated = false
end

function Sprite:draw()
	drystal.set_color(self.color)
	drystal.set_alpha(self.alpha)
	if self._updated then
		self:_update()
	end

	local x1, y1 = self._x1, self._y1
	local x2, y2 = self._x2, self._y2
	local x3, y3 = self._x3, self._y3
	local x4, y4 = self._x4, self._y4
	if self.source then
		local xi = self.source.x
		local yi = self.source.y
		local xi2 = self.source.x + self.source.w
		local yi2 = self.source.y + self.source.h

		if self.w < 0 then
			xi, xi2 = xi2, xi
		end
		if self.h < 0 then
			yi, yi2 = yi2, yi
		end

		_draw_quad(xi, yi, xi2, yi, xi2, yi2, xi, yi2,
				  x1, y1, x2,  y2, x3,  y3,  x4, y4)
	else
		drystal.draw_triangle(x1, y1, x2, y2, x3, y3)
		drystal.draw_triangle(x1, y1, x4, y4, x3, y3)
	end
end


local weird_shader_vertex = [[
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

function drystal.display_logo(sprite, background)
	background = background or {255, 255, 255}
	local duration = 6000
	local shader = assert(drystal.new_shader(weird_shader_vertex))
	if shader then
		shader:use()
	end
	local time = 0
	local _update = drystal.update
	local _draw = drystal.draw

	local wr, hr = drystal.screen.w, drystal.screen.h
	local ws = math.min(wr, sprite.w) - 50
	local hs = math.min(hr, sprite.h) - 50

	drystal.update = function(dt)
		time = time + dt
		if time > duration then
			drystal.update = _update
			drystal.draw = _draw
			drystal.set_alpha(255)
		end
	end
	local transform = {
		angle=0,
		wfactor=ws/sprite.w,
		hfactor=hs/sprite.h,
	}
	drystal.draw = function()
		if shader then shader:feed('tick', time*0.1) end

		drystal.set_alpha(255)
		drystal.set_color(unpack(background))
		drystal.draw_background()

		drystal.set_color(255, 255, 255)
		local a = 255 - (math.cos(time*math.pi/(duration/2))/2+0.5)*255
		drystal.set_alpha(a)
		drystal.draw_sprite(sprite, (wr-ws)/2, (hr-hs)/2, transform)
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

return drystal

