package.path = 'data/?.lua;' .. package.path
package.cpath = 'data/?.so;' .. package.cpath

require 'drystal'
local tt = require 'truetype'

function init()
	resize(512, 512)

	font = tt.load('data/arial.ttf', 16)
	font_big = tt.load('data/arial.ttf', 42)
end

function draw()
	set_alpha(255)
	set_color(200, 200, 200)
	draw_background()

	set_color(255, 0, 0)
	tt.use(font)
	local w = tt.sizeof('abcdefghijklmopqrstuvwxyz')
	tt.draw('abcdefghijklmopqrstuvwxyz', (512 - w) / 2, 100)

	tt.use(font_big)
	local text = 'abd {r0|b150|bla} {small|test} {big|50%|defghi}'
	local w, h = tt.sizeof(text)
	tt.draw_color(text, (512 - w) / 2, (512-h)/2)

	flip()
end

function key_press(k)
	tt.free(font)
	engine_stop()
end

