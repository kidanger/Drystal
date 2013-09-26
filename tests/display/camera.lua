local drystal = require 'drystal'

assert(drystal.camera.x == 0)
assert(drystal.camera.blo == nil)
drystal.camera.blo = "t"
assert(drystal.camera.blo == "t")

function drystal.init()
	drystal.resize(600, 400)
end

function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	drystal.set_color(255, 255, 255)
	drystal.draw_circle(300, 200, 10)

	drystal.flip()
end

function mouse_motion(x, y)
	drystal.camera.x = x
	drystal.camera.y = y
	drystal.camera.angle = math.random()*math.pi*2
	print(drystal.camera.x, drystal.camera.y)
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.engine_stop()
	end
end

