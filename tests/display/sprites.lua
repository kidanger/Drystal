local drystal = require 'drystal'

local spritesheet = drystal.deserialize(io.open('image.json'):read('*all'))
local image = assert(drystal.load_surface(spritesheet.meta.image))

local sprites = {}

function drystal.init()
	drystal.resize(600, 400)
	drystal.draw_from(image)
end

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

table.insert(sprites, drystal.Sprite {
	x=300,
	y=200,
	w=40,
	h=40,
	color={200, 0, 0},
	update=function(self, dt)
		self.angle = self.angle + dt * math.pi * 2
		self.color[1] = self.color[1] + dt * 255 * 2
		if self.color[1] > 255 then
			self.color[1] = 0
		end
	end
})

local sprite = spritesheet.frames['character.png'].frame
table.insert(sprites, drystal.Sprite {
	x=350,
	y=200,
	w=sprite.w,
	h=sprite.h,
	source=sprite,
	update=function(self, dt)
		self.angle = self.angle + dt * math.pi * 2
	end
})

