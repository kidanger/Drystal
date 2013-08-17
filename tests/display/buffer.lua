require "drystal"

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

width = 0
height = 0
function init()
	resize(600, 400)
	image = load_surface(spritesheet.meta.image)
	draw_from(image)
end

local bufferize = true
local buffer
local number = 6000
function draw()
	draw_from(image)
	set_alpha(255)
	set_color(10, 10, 30)
	draw_background()

	set_color(255, 0, 0)
	set_alpha(105)

	if bufferize then
		if not buffer then
			buffer = new_buffer(number * 6)
			use_buffer(buffer)
			heavy_draw(number)
			use_buffer()
		end
		draw_buffer(buffer)
	else
		heavy_draw(number)
	end

	flip()
end

function heavy_draw(number)
	local sprite = spritesheet.frames['character.png'].frame
	for x = 0, number-1 do
		set_color(200, 200, 200)
		set_alpha(255)
		draw_sprite(sprite, x, (x%30)*30)
	end
end

function key_press(key)
	if key == 'escape' then
		engine_stop()
	elseif key == 'b' then
		bufferize = not bufferize
	end
end

