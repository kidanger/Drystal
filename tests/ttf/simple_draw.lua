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
	local w = tt.sizeof('abcdefghijklmopqrstuvwxyz')
	tt.draw('abcdefghijklmopqrstuvwxyz', (512 - w) / 2, 100)
	print('bla', w)

	tt.use(font_big)
	local text = 'abd {r0|big|b150|bla} {small|test} {big|50%|defghi}'
	text = highlight(text, (math.sin(time/1000)/2+0.5)*#text + 1)
	local w, h = tt.sizeof(text)
	tt.draw_color(text, (512 - w) / 2, (512-h)/2)

	flip()
end

function key_press(k)
	tt.free(font)
	tt.free(font_big)
	engine_stop()
end

