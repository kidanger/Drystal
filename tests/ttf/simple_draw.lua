require 'drystal'
local tt = require 'truetype'

function init()
	resize(512, 512)

	font = tt.load('arial.ttf', 16)
	font_big = tt.load('arial.ttf', 42)
end

time = 0
function update(dt)
	time = time + dt
end

local function highlight(text, pos)
	return text:sub(0, pos-1) .. '{big|'.. text:sub(pos, pos) .. '}' .. text:sub(pos+1, #text)
end

function draw()
	set_alpha(255)
	set_color(200, 200, 200)
	draw_background()

	set_color(255, 0, 0)
	tt.use(font)
	tt.draw_align('abcdefghijklmopqrstuvwxyz', 512 / 2, 100, 'center')

	tt.use(font_big)
	tt.use_color(true)

	local text = 'abd {r0|big|b150|bla} {small|test} {big|50%|defghi}'
	text = highlight(text, (math.sin(time/1000)/2+0.5)*#text + 1)
	tt.draw_align(text, 512 / 2, 512 / 2, 'center')

	tt.use_color(false)
	tt.use(font)
	tt.draw_align(text, 512 / 2, 512 * 0.7, 'center')

	flip()
end

function key_press(k)
	if k == 'a' then
		tt.free(font)
		tt.free(font_big)
		engine_stop()
	end
end

