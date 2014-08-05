local drystal = require 'drystal'
local color = drystal.colors.green
function drystal.init()
	drystal.resize(600, 400)
	print('press d or l or m or a')
end

function drystal.update(dt)
end

function drystal.draw()
	drystal.set_color(color)
	drystal.draw_background()
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
