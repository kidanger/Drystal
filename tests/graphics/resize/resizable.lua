local drystal = require 'drystal'

local w, h = 600, 400

function drystal.init()
	print('init')
	drystal.resize(w, h)
	update_surf()
end

function drystal.draw()
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	drystal.set_color(0, 0, 0)
	drystal.draw_rect(2, 2, w - 4, h - 4)
end

function drystal.key_press(key)
	if key == 'h' then
		w = w - 100
	elseif key == 'l' then
		w = w + 100
	elseif key == 'j' then
		h = h + 100
	elseif key == 'k' then
		h = h - 100
	else
		return
	end
	drystal.resize(w, h)
	update_surf()
end

function update_surf()
	w, h = drystal.screen.w, drystal.screen.h
	print("hjlk to resize, current size is " .. w .. "x" .. h)
end

