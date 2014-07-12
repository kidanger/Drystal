local drystal = require 'drystal'

drystal.resize(600, 400)

local gray = false
local red = false

local time = 0
function drystal.update(dt)
	time = time + dt
end

function drystal.draw(dt)
	drystal.set_alpha(255)
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	drystal.set_color(255, 0, 0)
	drystal.draw_rect(50, 50, 300, 200)

	drystal.set_color(0, 0, 100)
	drystal.set_alpha(200)
	drystal.draw_rect(10, 10, 100, 100)

	drystal.set_color(0, 255, 100)
	drystal.set_alpha(255)
	drystal.draw_rect(100, 100, 100, 100)

	if gray then
		drystal.postfx('gray')
	end
	if red then
		drystal.postfx('red', math.sin(time * 10)/2 + .5)
	end
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	elseif k == 'g' then
		gray = not gray
	elseif k == 'r' then
		red = not red
	end
end
