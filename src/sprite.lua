local drystal = require 'drystal'

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

drystal.Sprite = Sprite

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

		drystal.draw_quad(xi, yi, xi2, yi, xi2, yi2, xi, yi2,
                            x1, y1, x2,  y2, x3,  y3,  x4, y4)
	else
		drystal.draw_triangle(x1, y1, x2, y2, x3, y3)
		drystal.draw_triangle(x1, y1, x4, y4, x3, y3)
	end
end

