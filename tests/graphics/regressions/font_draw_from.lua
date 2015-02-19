local drystal = require 'drystal'

drystal.resize(600, 400)

local x, y = 0, 0
local font = assert(drystal.load_font('arial.ttf', 25))
local text = 'all fine!'
local surface = drystal.new_surface(font:sizeof(text))
local sprite = drystal.new_sprite({x=0, y=0, w=surface.w, h=surface.h}, x, y)
sprite.angle = math.pi * 2

function drystal.draw()
	collectgarbage()
	drystal.set_color(250, 20, 20)
	drystal.set_alpha(255)
	drystal.draw_background()

	local screen = surface:draw_on()
	drystal.set_color(0, 0, 0)
	font:draw(text, 0, 0)
	surface:draw_from()
	screen:draw_on()

	sprite:draw()
end
