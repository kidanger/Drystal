local drystal = require 'drystal'

drystal.resize(600, 400)

local spritesheet = assert(drystal.fromjson(io.open('image.json'):read('*all')))
local sprite = spritesheet.frames['character.png'].frame

image = assert(drystal.load_surface(spritesheet.meta.image))
image:draw_from()

local sprites = {}

function drystal.update(dt)
	for _, s in ipairs(sprites) do
		s:update(dt)
	end
end

function drystal.draw()
	drystal.set_color(20, 20, 20)
	drystal.draw_background()

	for _, s in ipairs(sprites) do
		s:draw()
	end
end

local s1 = drystal.new_sprite(sprite, 300, 200, 40, 40)
s1.color = drystal.colors.red
s1.update = function(self, dt)
	self.angle = self.angle + dt * math.pi * 2
end

local s2 = drystal.new_sprite(sprite, 150, 200, 100, 100)
s2.update=function(self, dt)
	self.angle = self.angle + dt * math.pi * 2
	self.color[1] = self.color[1] + dt * 255 * 2
	if self.color[1] > 255 then
		self.color[1] = 0
	end
end

table.insert(sprites, s1)
table.insert(sprites, s2)

