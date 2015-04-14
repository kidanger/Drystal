local drystal = require 'drystal'

assert(drystal.camera.x == 0)
assert(drystal.camera.y == 0)
assert(drystal.camera.angle == 0)
assert(drystal.camera.zoom == 1)

assert(drystal.camera.blo == nil)
drystal.camera.blo = "t"
assert(drystal.camera.blo == "t")

function drystal.init()
	drystal.resize(600, 400)
end

local mx, my = 0, 0
function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	drystal.set_color(255, 255, 255)
	drystal.draw_circle(300, 200, 10)

	drystal.camera.push()
	drystal.camera.reset()
	drystal.set_color(255, 255, 255)
	drystal.draw_circle(300, 200, 10)
	drystal.camera.pop()

	drystal.camera.push()
	drystal.camera.reset()
	drystal.camera.zoom = 0.3
	drystal.camera.angle = math.pi / 4
	drystal.camera.x = 200
	drystal.camera.y = -20

	drystal.set_color 'blue'
	drystal.draw_square(0, 0, drystal.screen.w, drystal.screen.h)
	local mxx, myy = drystal.screen2scene(mx, my)
	drystal.set_color 'green'
	drystal.draw_circle(mx, my, 40)
	drystal.camera.pop()

	drystal.set_color 'blue'
	drystal.camera.push()
	drystal.camera.reset()
	drystal.draw_circle(mxx, myy, 9)
	drystal.camera.pop()
end

function drystal.mouse_motion(x, y)
	drystal.camera.x = 300 - x
	drystal.camera.y = 200 - y
	drystal.camera.angle = drystal.camera.angle + math.pi/32

	mx = x
	my = y
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

