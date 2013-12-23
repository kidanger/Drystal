local drystal = require "drystal"

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

function drystal.init()
	drystal.resize(600, 400)
	image = drystal.load_surface(spritesheet.meta.image)
	drystal.draw_from(image)
	print('b to toggle buffer')
end

local bufferize = true
local buffer
local number = 6000
local tick = 0
function drystal.draw()
	tick = tick + 1

	drystal.draw_from(image)
	drystal.set_alpha(255)
	drystal.set_color(10, 10, 30)
	drystal.draw_background()

	local sprite = spritesheet.frames['character.png'].frame
	drystal.set_color(0, 255, 0)
	drystal.set_alpha(255)
	drystal.draw_sprite(sprite, 300, 200)

	drystal.set_color(255, 0, 0)
	drystal.set_alpha(105)

	if bufferize then
		if not buffer then
			buffer = drystal.new_buffer(number * 6)
			drystal.use_buffer(buffer)
			heavy_draw(number)
			drystal.use_buffer()
		end
		drystal.draw_buffer(buffer, math.sin(tick/10)*50)
	else
		heavy_draw(number)
	end

	local sprite = spritesheet.frames['character.png'].frame
	drystal.set_color(255, 0, 0)
	drystal.set_alpha(255)
	drystal.draw_sprite(sprite, 332, 200)
	drystal.draw_line(0, 0, 600, 400)
end

function heavy_draw(number)
	local sprite = spritesheet.frames['character.png'].frame
	for x = 0, number-1 do
		drystal.set_color(200, 200, 200)
		drystal.set_alpha(255)
		drystal.draw_sprite(sprite, x, (x%30)*30)
	end
end

function drystal.key_press(key)
	if key == 'a' then
		drystal.stop()
	elseif key == 'b' then
		bufferize = not bufferize
	end
end

