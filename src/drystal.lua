local drystal = require 'drystal'

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
	sprite.w = (sprite.w > 0 and sprite.w) or (sprite.source and sprite.source.w) or 0
	sprite.h = (sprite.h > 0 and sprite.h) or (sprite.source and sprite.source.h) or 0
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

drystal.postfxs = {}
local backsurface

function drystal.create_postfx(name, code, uniforms)
	uniforms = uniforms or {}
	local uniforms_code = ''
	for i, name in ipairs(uniforms) do
		uniforms_code = uniforms_code .. [[
			uniform float ]] .. name .. [[;
		]]
	end
	code = [[
		varying vec2 fTexCoord;
		uniform sampler2D tex;
		]] .. uniforms_code .. [[
		]] .. code .. [[
		void main()
		{
			gl_FragColor = vec4(effect(tex, fTexCoord), 1.0);
		}
	]]
	--print(code)
	local shader = assert(drystal.new_shader(nil, nil, code))
	local fx = function(...)
		local screen = drystal.screen

		drystal.set_color(255, 255, 255)
		drystal.set_alpha(255)

		shader:use()
		for i, u in ipairs(uniforms) do
			local v = select(i, ...) or 0
			shader:feed(u, v)
		end
		local old = screen:draw_from()
		backsurface:draw_on()
		drystal.draw_image(0, 0, backsurface.w, backsurface.h, 0, 0)

		drystal.use_shader()
		backsurface:draw_from()
		screen:draw_on()
		drystal.draw_image(0, 0, backsurface.w, backsurface.h, 0, 0)

		if old then
			old:draw_from()
		end
	end
	drystal.postfxs[name] = fx
end

drystal.create_postfx('gray', [[
	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec3 texval = texture2D(tex, coord).rgb;
		return mix(texval, vec3((texval.r + texval.g + texval.b) / 3.0), scale);
	}
]], {'scale',})

drystal.create_postfx('red', [[
	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec3 texval = texture2D(tex, coord).rgb;
		return vec3(red, 0., 0.) + texval;
	}
]], {'red',})

drystal.create_postfx('distortion', [[
	#define pi ]] .. math.pi .. [[

	vec3 effect(sampler2D tex, vec2 coord)
	{
		coord.x += sin(coord.y * 8.*pi + time * 2. * pi * .75) / 100.;
		return texture2D(tex, coord).rgb;
	}
]], {'time',})

drystal.create_postfx('blurDir', [[
	const float weight1 = 0.3989422804014327;
	const float weight2 = 0.24197072451914536;
	const float weight3 = 0.05399096651318985;
	const float weight4 = 0.004431848411938341;

	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec2 dir = vec2(dx, dy);
		vec3 acc = vec3(0., 0., 0.);

		acc += texture2D(tex, coord).rgb * weight1;

		acc += texture2D(tex, coord + dir).rgb * weight2;
		acc += texture2D(tex, coord - dir).rgb * weight2;

		acc += texture2D(tex, coord + dir*2.).rgb * weight3;
		acc += texture2D(tex, coord - dir*2.).rgb * weight3;

		acc += texture2D(tex, coord + dir*3.).rgb * weight4;
		acc += texture2D(tex, coord - dir*3.).rgb * weight4;

		acc /= weight1 + (weight2 + weight3 + weight4) * 2.;
		return acc;
	}
]], {'dx', 'dy',})

drystal.postfxs.blur = function(...)
	drystal.postfxs.blurDir(1 / drystal.screen.w, 0)
	drystal.postfxs.blurDir(0, 1 / drystal.screen.h)
end

drystal.create_postfx('vignette', [[
	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec2 m = vec2(0.5, 0.5);
		float d = distance(m, coord);
		vec3 texval = texture2D(tex, coord).rgb;
		return texval * smoothstep(outer, inner, d);
	}
]], {'outer', 'inner',})

drystal.create_postfx('dither', [[
	/*
		Port of shader by Ceaphyrel, found at
		http://www.assembla.com/code/MUL2010_OpenGLScenePostprocessing/subversion/nodes/MUL%20FBO/Shaders/dithering.frag?rev=83
		toneburst 2011
	*/
	float find_closest(int x, int y, float c0)
	{
		mat4 dither = mat4(
			1.0,  33.0,  9.0, 41.0,
			49.0, 17.0, 57.0, 25.0,
			13.0, 45.0,  5.0, 37.0,
			61.0, 29.0, 53.0, 21.0 );
		
		float limit = 0.0;
		for (int xx = 0; xx < 8; xx++)
		{
			if (xx == x)
			{
				for (int yy = 0; yy < 8; yy++)
				{
					if (yy == y)
					{
						if(x < 4) {
							if(y >= 4) {
								limit = (dither[xx][yy-4]+3.0)/65.0;
							} else {
								limit = (dither[xx][yy])/65.0;
							}
						}
						if(x >= 4) {
							if(y >= 4)
								limit = (dither[xx-4][yy-4]+1.0)/65.0;
							else
								limit = (dither[xx-4][yy]+2.0)/65.0;
						}
						break;
					}
				}
				break;
			}
		}
		if(c0 < limit)
			return 0.0;
		
		return 1.0;
	}

	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec4 texval = texture2D(tex, coord.xy);
		float grayscale = dot(texval, vec4(0.299, 0.587, 0.114, 0));
		vec2 xy = coord * scale;

		int x = int(mod(xy.x, 8.0));
		int y = int(mod(xy.y, 8.0));
		float final = find_closest(x, y, grayscale);
		return final * texval.rgb;
	}
]], {'scale',})

function drystal.postfx(name, ...)
	if not drystal.postfxs[name] then
		error('Post FX ' .. name .. ' not found.')
	end
	local screen = drystal.screen
	if not backsurface or backsurface.w ~= screen.w or backsurface.h ~= screen.h then
		backsurface = drystal.new_surface(screen.w, screen.h, true)
	end
	drystal.postfxs[name](...)
end

