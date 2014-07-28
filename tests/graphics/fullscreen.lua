local drystal = require 'drystal'

drystal.resize(800, 600)
function drystal.draw(dt)
	drystal.set_alpha(255)
	drystal.set_color(255, 25, 255)
	drystal.draw_background()

	drystal.set_color(255, 0, 0)
	drystal.draw_rect(50, 50, 300, 200)
end

function drystal.key_press(k)
	if k == 'f' then
		drystal.set_fullscreen(true)
	elseif k == 'g' then
		drystal.set_fullscreen(false)
	end
end
