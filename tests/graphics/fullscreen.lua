local drystal = require 'drystal'

drystal.resize(800, 600)
function drystal.draw(dt)
	drystal.set_alpha(255)
	drystal.set_color(255, 25, 255)
	drystal.draw_background()

	drystal.set_color(255, 0, 0)
	drystal.draw_rect(50, 50, drystal.screen.w - 100, drystal.screen.h - 100)
end

local full = false
function drystal.key_press(k)
	if k == 'f' then
		full = true
		drystal.set_fullscreen(true)
	elseif k == 'g' then
		full = false
		drystal.set_fullscreen(false)
	end
end

function drystal.page_resize(w, h)
	print(w, h)
	if full then
		drystal.set_fullscreen(true)
	end
end

