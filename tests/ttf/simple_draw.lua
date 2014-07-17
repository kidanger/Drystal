local drystal = require 'drystal'

local font, font_big
function drystal.init()
	drystal.resize(512, 512)

	font = drystal.load_font('arial.ttf', 16)
	font_big = drystal.load_font('arial.ttf', 42)
end

local time = 0
function drystal.update(dt)
	time = time + dt
end

local function highlight(text, pos)
	return text:sub(0, pos-1) .. '{big|'.. text:sub(pos, pos) .. '}' .. text:sub(pos+1, #text)
end

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(200, 200, 200)
	drystal.draw_background()

	drystal.set_color(255, 0, 0)
	font:draw_align('abcdefghijklmopqrstuvwxyz', 512 / 2, 100, 'center')

	local text = 'abd {r:0|big|b:150|bla} {small|test} {big|%:50|defghi}'
	text = highlight(text, (math.sin(time)/2+0.5)*#text + 1)
	font_big:draw_align(text, 512 / 2, 512 / 2, 'center')

	font:draw_plain_align(text, 512 / 2, 512 * 0.7, 'center')
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

