require "drystal"
local font = require "truetype"

function init()
	resize(600, 400);
	font.use(assert(font.load('arial.ttf', 20)))
end

function draw()
	set_color(255, 255, 255)
	draw_background()

	local text = 'blajblaj'
	local w, h = font.sizeof(text)
	set_color(0, 0, 0)
	draw_square(10, 10, w, h)
	font.draw(text, 10, 10)

	flip()
end
