local drystal = require 'drystal'

function drystal.init()
	drystal.resize(800, 600)
end

function drystal.update(dt)
end

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	drystal.set_color(0, 0, 0)
	drystal.draw_rect(300, 300, 50, 50)
	drystal.draw_rect(200, 200, 1, 1)
	drystal.draw_rect(204, 204, 2, 2)

	drystal.flip()
end

function drystal.mouse_motion(x, y)
	drystal.camera.x = x - 400
	drystal.camera.y = y - 300
end
function drystal.mouse_press(x, y, b)
	if b == 4 then
		drystal.camera.zoom = drystal.camera.zoom * 1.3
	elseif b == 5 then
		drystal.camera.zoom = drystal.camera.zoom / 1.3
	end
end
