require 'drystal'

function init()
	resize(600, 400)
end

local time = 0
function update(dt)
	time = time + dt
end

function draw()
	set_color(255, 255, 255)
	draw_background()

	for y = 0, 400, 8 do
		for x = 0, 600, 8 do
			local r = math.sin((x + y + time)/100)/2+0.5
			set_point_size(8)
			set_color(r*255, 0, 0)
			draw_point(x, y)
		end
	end

	flip()
end

function key_press(k)
	if k == 'a' then
		stop_engine()
	end
end
