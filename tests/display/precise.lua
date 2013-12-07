local drystal = require 'drystal'

local w, h = 32, 32
local surface
local image
local image_sprite = {x=16, y=16, w=32, h=32}
function drystal.init()
	drystal.resize(800, 600)
	drystal.camera.reset()
	drystal.set_filter_mode(drystal.FILTER_NEAREST)

	image = drystal.load_surface('precise.png')

	surface = drystal.new_surface(w, h)
	drystal.draw_on(surface)
	drystal.set_color(255, 0, 0)
	drystal.draw_rect(0, 0, w, h)

	local s2 = drystal.new_surface(w-2, h-2)
	drystal.draw_on(s2)
	drystal.set_color(0, 255, 0)
	drystal.draw_rect(0, 0, w-2, h-2)

	drystal.draw_from(s2)
	drystal.draw_on(surface)
	drystal.set_color(255, 255, 255)
	drystal.draw_sprite({x=0, y=0, w=w-2, h=h-2}, 1, 1)

	drystal.draw_on(drystal.screen)
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

	drystal.draw_rect(150-w, 150, w, h)

	drystal.draw_from(surface)
	drystal.set_color(255, 255, 255)
	drystal.draw_sprite({x=0, y=0, w=w, h=h}, 150, 150)

	drystal.draw_from(image)
	drystal.set_color(255, 255, 255)
	drystal.draw_sprite(image_sprite, 150+w, 150)

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

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

