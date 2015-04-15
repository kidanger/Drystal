local drystal = require 'drystal'

if test == nil then
	test = false
	assert(drystal.camera.x == 0)
	assert(drystal.camera.y == 0)
	assert(drystal.camera.angle == 0)
	assert(drystal.camera.zoom == 1)

	assert(drystal.camera.blo == nil)
	drystal.camera.blo = "t"
	assert(drystal.camera.blo == "t")
end

function drystal.init()
	drystal.resize(600, 400)
end

local angle = 0
local function setupcam()
	drystal.camera.reset()
	angle = angle + 0.01
	drystal.camera.zoom = 0.9
	drystal.camera.angle = angle
	drystal.camera.x = 300
	drystal.camera.y = 200
end

local mx, my = 0, 0
local cx, cy = 0, 0
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
	setupcam()

	local x1, y1 = drystal.screen2scene(100, 100)
	local x2, y2 = drystal.screen2scene(500, 100)
	local x3, y3 = drystal.screen2scene(500, 300)
	local x4, y4 = drystal.screen2scene(100, 300)

	drystal.set_color 'green'
	drystal.draw_circle(mx, my, 13/drystal.camera.zoom)

	drystal.set_color 'lime'
	drystal.camera.pop()
	drystal.camera.push()
	drystal.camera.reset()
	drystal.draw_line(x1, y1, x2, y2)
	drystal.draw_line(x2, y2, x3, y3)
	drystal.draw_line(x3, y3, x4, y4)
	drystal.draw_line(x4, y4, x1, y1)
	drystal.draw_circle(x1, y1, 10)
	drystal.set_color 'orange'
	drystal.draw_circle(x2, y2, 10)
	drystal.set_color 'chocolate'
	drystal.draw_circle(x3, y3, 10)
	drystal.set_color 'brown'
	drystal.draw_circle(x4, y4, 10)
	drystal.camera.pop()

	drystal.set_color 'blue'
	drystal.camera.push()
	drystal.camera.reset()
	drystal.draw_circle(cx, cy, 9)
	drystal.camera.pop()
end

function drystal.mouse_motion(x, y)
	drystal.camera.x = 300 - x
	drystal.camera.y = 200 - y
	drystal.camera.angle = drystal.camera.angle + math.pi/32

	mx = x
	my = y

	drystal.camera.push()
	setupcam()
	cx, cy = drystal.screen2scene(mx, my)
	drystal.camera.pop()
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

