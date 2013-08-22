require 'drystal'

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

local mx, my = 0, 0

function init()
	resize(600, 400)
	image = load_surface(spritesheet.meta.image)
	draw_from(image)
end

local time = 0
function update(dt)
	time = time + dt
end

function draw()
	set_color(255, 255, 255)
	draw_background()

	for y = 0, 400, 8 do
		for x = 0, 600, 8 do
			local r = math.sin((x + y + time)/100)/2+0.5
			set_point_size(8)
			set_color(r*255, 0, 0)
			draw_point(x, y)
		end
	end

	set_point_size(32)
	set_color(0, 255, 0)
	draw_point(mx, my)

	set_point_size(16)
	local sprite = spritesheet.frames['character.png'].frame
	draw_point_tex(sprite.x+sprite.w/2, sprite.y+sprite.h/2, mx, my)

	set_line_width(5)
	draw_line(0, 0, mx, my)

	flip()
end

function mouse_motion(x, y)
	mx = x
	my = y
end

function key_press(k)
	if k == 'a' then
		engine_stop()
	end
end
