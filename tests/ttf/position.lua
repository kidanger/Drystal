local drystal = require "drystal"
local font = require "truetype"

function drystal.init()
	drystal.resize(600, 400);
	font.use(assert(font.load('arial.ttf', 20)))
end

function drystal.draw()
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	local text = 'blajblaj'
	local w, h = font.sizeof(text)
	drystal.set_color(0, 0, 0)
	drystal.draw_square(10, 10, w, h)
	font.draw(text, 10, 10)

	drystal.flip()
end
