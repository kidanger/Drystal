local drystal = require 'drystal'

local image

function drystal.init()
	drystal.resize(600, 400)
	image = drystal.load_surface('tex.png')
end

function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.set_alpha(255)
	drystal.draw_background()

	local radius = 3
	for x = 1, image.w do
		for y = 1, image.h do
			local r, g, b, a = image:get_pixel(x, y)
			drystal.set_color(r, g, b)
			drystal.set_alpha(a)
			drystal.draw_point(x * radius, y * radius, radius)
		end
	end
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

