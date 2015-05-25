local drystal = require 'drystal'

drystal.resize(600, 400)

local mx = 0
local my = 0

function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	drystal.set_color(255, 255, 255)
	drystal.draw_line(300, 200, mx, my, 6)
end

function drystal.mouse_motion(x, y)
	mx = x
	my = y
end
