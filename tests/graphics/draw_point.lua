local drystal = require 'drystal'

local mx, my = 0, 0

function drystal.init()
	drystal.resize(600, 400)
	image = drystal.load_surface('tex.png')
	image:draw_from()
end

local time = 0
function drystal.update(dt)
	time = time + dt
end

function drystal.draw()
	drystal.set_color(drystal.colors.white)
	drystal.draw_background()

	for y = 0, 400, 8 do
		for x = 0, 600, 8 do
			local r = math.sin((x + y + time*1000)/100)/2+0.5
			drystal.set_color(r*255, 0, 0)
			drystal.draw_point(x, y, 8)
		end
	end

	drystal.set_color(drystal.colors.lime)
	drystal.draw_point_tex(mx-8, my-8, 15)
end

function drystal.mouse_motion(x, y)
	mx = x
	my = y
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end
