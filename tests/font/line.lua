local drystal = require 'drystal'

drystal.resize(600, 400)

local font = assert(drystal.load_font('arial.ttf', 20))

local text = [[blba
dsqdsq dsq
k{g:40|??{outr:80|outline|ds}lol}qj dk{r:50|sq
ds}qj dksq
]]

function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	drystal.set_color(255, 255, 255)

	font:draw_plain(text, 10, 10)
	drystal.draw_square(10, 10, font:sizeof_plain(text))

	font:draw(text, 10, 100)
	drystal.draw_square(10, 100, font:sizeof(text))

	font:draw(text, 100, 200, drystal.aligns.center)
	drystal.draw_line(100, 200, 100, 300)
	font:draw(text, 300, 200, drystal.aligns.right)
	drystal.draw_line(300, 200, 300, 300)
end

