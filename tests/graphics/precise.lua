local drystal = require 'drystal'

local w, h = 32, 32
local surface
local image
local image_sprite = {x=16, y=16, w=32, h=32}
function drystal.init()
	drystal.resize(800, 600)
	drystal.camera.reset()

	image = assert(drystal.load_surface('precise.png'))
	image:set_filter(drystal.filters.nearest)

	surface = drystal.new_surface(w, h)
	surface:set_filter(drystal.filters.nearest)
	surface:draw_on()
	drystal.set_color(255, 0, 0)
	drystal.draw_rect(0, 0, w, h)

	local s2 = assert(drystal.new_surface(w-2, h-2))
	s2:set_filter(drystal.filters.nearest)
	s2:draw_on()
	drystal.set_color(0, 255, 0)
	drystal.draw_rect(1, 1, w-2, h-2)

	s2:draw_from()
	surface:draw_on()
	drystal.set_color(255, 255, 255)
	drystal.draw_sprite({x=0, y=0, w=w-2, h=h-2}, 1, 1)

	drystal.screen:draw_on()
end

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	drystal.set_color(0, 0, 0)
	drystal.draw_rect(300, 300, 50, 50)
	drystal.draw_rect(200, 200, 1, 1)
	drystal.draw_rect(204, 204, 2, 2)

	drystal.draw_rect(150-w, 150, w, h)

	surface:draw_from()
	drystal.set_color(255, 255, 255)
	drystal.draw_sprite({x=0, y=0, w=w, h=h}, 150, 150)

	image:draw_from()
	drystal.set_color(255, 255, 255)
	drystal.draw_sprite(image_sprite, 150+w, 150)
end

function drystal.mouse_motion(x, y)
	drystal.camera.x = 400 - x
	drystal.camera.y = 300 - y
end
function drystal.mouse_press(x, y, b)
	if b == drystal.buttons.wheel_up then
		drystal.camera.zoom = drystal.camera.zoom * 1.3
	elseif b == drystal.buttons.wheel_down then
		drystal.camera.zoom = drystal.camera.zoom / 1.3
	end
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

