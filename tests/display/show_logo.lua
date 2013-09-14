local drystal = require "drystal"

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

function drystal.init()
	drystal.resize(600, 400)
	image = drystal.load_surface(spritesheet.meta.image)
end

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(10, 10, 30)
	drystal.draw_background()

	drystal.draw_from(image)
	local sprite = spritesheet.frames['logo.png'].frame
	drystal.set_color(200, 200, 200)
	drystal.set_alpha(255)
	drystal.draw_sprite(sprite, 0, 0)

	drystal.set_color(255, 128, 0)
	drystal.set_alpha(100)
	local surf = drystal.new_surface(10, 10)
	drystal.draw_on(surf)
	drystal.draw_rect(0, 0, 10, 10)
	drystal.draw_on(screen)
	drystal.draw_from(surf)
	sprite = {x=0, y=0, w=10, h=10}
	drystal.draw_sprite(sprite, 500, 310)
	drystal.free_surface(surf)

	drystal.flip()
end

function drystal.key_press(key)
	if key == 'escape' then
		drystal.engine_stop()
	end
end

