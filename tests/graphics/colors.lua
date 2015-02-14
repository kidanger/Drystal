local drystal = require 'drystal'

local color = drystal.colors['#22DD22']

function drystal.init()
	drystal.resize(600, 400)
	print('press d or l or m or a')
end

function drystal.update(dt)
end

function drystal.draw()
	drystal.set_color(color)
	drystal.draw_background()

	drystal.set_color 'cyan'
	drystal.draw_rect(20, 20, 40, 40)
end

function drystal.key_press(key)
	if key == 'd' then
		color = color:darker()
	end
	if key == 'l' then
		color = color:lighter()
	end
	if key == 'm' then
		color = color * drystal.colors.steelblue
	end
	if key == 'a' then
		color = color + drystal.colors.red
	end
end
