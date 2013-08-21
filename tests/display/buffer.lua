require "drystal"

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

function init()
	resize(600, 400)
	image = load_surface(spritesheet.meta.image)
	draw_from(image)
end

local bufferize = true
local buffer
local number = 6000
local tick = 0
function draw()
	tick = tick + 1

	draw_from(image)
	set_alpha(255)
	set_color(10, 10, 30)
	draw_background()

	local sprite = spritesheet.frames['character.png'].frame
	set_color(0, 255, 0)
	set_alpha(255)
	draw_sprite(sprite, 300, 200)

	set_color(255, 0, 0)
	set_alpha(105)

	if bufferize then
		if not buffer then
			buffer = new_buffer(number * 6)
			use_buffer(buffer)
			heavy_draw(number)
			use_buffer()
		end
		draw_buffer(buffer, math.sin(tick/10)*50)
	else
		heavy_draw(number)
	end

	local sprite = spritesheet.frames['character.png'].frame
	set_color(255, 0, 0)
	set_alpha(255)
	draw_sprite(sprite, 332, 200)
	draw_line(0, 0, 600, 400)

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

