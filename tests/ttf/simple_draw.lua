package.path = 'data/?.lua;' .. package.path
package.cpath = 'data/?.so;' .. package.cpath

require 'drystal'
require 'truetype'

function init()
	resize(512, 512)

	font = load_font('data/arial.ttf', 16)
	font_big = load_font('data/arial.ttf', 42)
end

function draw()
	set_alpha(255)
	set_color(200, 200, 200)
	draw_background()

	set_color(255, 0, 0)
	use_font(font)
	draw_text('abcdefghijklmopqrstuvwxyz', 50, 100)

	use_font(font_big)
	draw_text('abcdefghijklmopqrstuvwxyz', 50, 150)

	flip()
end

function key_press(k)
	free_font(font)
	engine_stop()
end

