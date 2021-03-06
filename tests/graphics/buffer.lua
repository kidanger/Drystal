local drystal = require "drystal"

-- test sizes
buffer = drystal.new_buffer(6)
buffer:use()
drystal.draw_point(1,1,4)
drystal.draw_point(1,1,4)
drystal.draw_point(1,1,4)
drystal.draw_point(1,1,4)
drystal.draw_point(1,1,4)
drystal.draw_point(1,1,4)

buffer = assert(drystal.new_buffer(4))
buffer:use()
drystal.draw_line(1,1,1,1)
drystal.draw_line(1,1,1,1)

buffer = assert(drystal.new_buffer(10))
buffer:use()
drystal.draw_triangle(1,1,1,1,1,1)
drystal.draw_triangle(1,1,1,1,1,1)
drystal.draw_triangle(1,1,1,1,1,1)

drystal.use_default_buffer()

local spritesheet = assert(drystal.fromjson(io.open('image.json'):read('*all')))

function drystal.init()
	drystal.resize(600, 400)
	image = assert(drystal.load_surface(spritesheet.meta.image))
	image:draw_from()
	print('b to toggle buffer')
end

local bufferize = true
local buffer
local number = 3000
local tick = 0
function drystal.draw()
	tick = tick + 1

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
			buffer = assert(drystal.new_buffer(number * 6 / 2))
			buffer:use()
			heavy_draw(number)
			buffer:upload_and_free()
			drystal.use_default_buffer()
		end
		buffer:draw(math.sin(tick/10)*sprite.w)
	else
		heavy_draw(number)
	end

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
	elseif key == 'f' then
		drystal.set_fullscreen(true)
	end
end

