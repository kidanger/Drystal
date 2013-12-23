local drystal = require 'drystal'

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

local mx, my = 0, 0

function drystal.init()
	drystal.resize(600, 400)
	image = drystal.load_surface(spritesheet.meta.image)
	drystal.draw_from(image)
end

local time = 0
function drystal.update(dt)
	time = time + dt
end

function drystal.draw()
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	for y = 0, 400, 8 do
		for x = 0, 600, 8 do
			local r = math.sin((x + y + time)/100)/2+0.5
			drystal.set_point_size(8)
			drystal.set_color(r*255, 0, 0)
			drystal.draw_point(x, y)
		end
	end

	drystal.set_point_size(32)
	drystal.set_color(0, 255, 0)
	drystal.draw_point(mx, my)

	drystal.set_point_size(16)
	local sprite = spritesheet.frames['character.png'].frame
	drystal.draw_point_tex(sprite.x+sprite.w/2, sprite.y+sprite.h/2, mx, my)

	drystal.set_line_width(5)
	drystal.draw_line(0, 0, mx, my)
end

function drystal.mouse_motion(x, y)
	mx = x
	my = y
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end
