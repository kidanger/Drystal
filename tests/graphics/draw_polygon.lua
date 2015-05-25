local drystal = require "drystal"

local mouse_points = {
}

function drystal.init()
	drystal.resize(600, 400)
end

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	drystal.set_color(0, 0, 0)
	local points = {
		25, 25,
		50, 25,
		75, 50,
		25, 50,
		25, 25
	}

	drystal.draw_polygon(unpack(points))

	drystal.set_color(200, 0, 0)
	drystal.draw_polyline(false, 5, unpack(points))

	if #mouse_points > 2 then
		drystal.draw_polygon(unpack(mouse_points))
		drystal.set_color(255, 0, 0)
		drystal.draw_polyline(true, unpack(mouse_points))
	end
end

function drystal.mouse_press(x, y, b)
	table.insert(mouse_points, x)
	table.insert(mouse_points, y)
end

function drystal.key_press(key)
	if key == 'a' then
		drystal.stop()
	end
end

