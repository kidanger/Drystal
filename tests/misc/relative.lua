local drystal = require 'drystal'

drystal.resize(800, 600)

local relative = false

local mx = 0
local my = 0
function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	drystal.set_color(255, 255, 255)
	drystal.draw_circle(mx, my, 10)
end

function drystal.mouse_motion(x, y, dx, dy)
	--print(x, y, dx, dy)
	if not relative then
		mx = x
		my = y
	else
		mx = mx + dx
		my = my + dy
		if mx < 0 then mx = 0 end
		if mx >= drystal.screen.w then mx = drystal.screen.w - 1 end
		if my < 0 then my = 0 end
		if my >= drystal.screen.h then my = drystal.screen.h - 1 end
	end
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	elseif k == 'space' then
		relative = not relative
		drystal.set_relative_mode(relative)
		print(relative)
	end
end

--drystal.run_js [[
	--var canvas = document.getElementById('canvas');
	--canvas.onmouseover = function() {
		--canvas.requestPointerLock();
	--};
--]]
