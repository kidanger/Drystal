local drystal = require 'drystal'

function drystal.init()
	drystal.resize(600, 400)
end

local time = 0
local red = 255
function drystal.update(dt)
	time = time + dt
	red = (math.sin(red) * .5 + .5) * 255
end

function drystal.draw()
	drystal.set_color(red, 0, 0)
	drystal.draw_background()
end

