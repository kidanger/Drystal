require "drystal"

local mouse_points = {
}

function init()
	resize(600, 400)
end

function update(dt)
end

function draw()
	set_alpha(255)
	set_color(255, 255, 255)
	draw_background()

	set_color(0, 0, 0)
	local points = {
		25, 25,
		50, 25,
		75, 50,
		25, 50,
		25, 25
	}

	draw_polyline(false, unpack(points))

	push_offset(200, 200)
	draw_polygon(unpack(points))
	pop_offset()

	if #mouse_points > 2 then
		draw_polygon(unpack(mouse_points))
		set_color(255, 0, 0)
		draw_polyline(true, unpack(mouse_points))
	end

	flip()
end

function mouse_press(x, y, b)
	table.insert(mouse_points, x)
	table.insert(mouse_points, y)
end

function key_press(key)
	if key == 'a' then
		engine_stop()
	end
end

