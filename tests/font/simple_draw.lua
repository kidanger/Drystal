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
	pos = math.floor(pos)
	return text:sub(0, pos-1) .. '{big|'.. text:sub(pos, pos) .. '}' .. text:sub(pos+1, -1)
end

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(200, 200, 200)
	drystal.draw_background()

	drystal.set_color(255, 0, 0)
	font:draw('abcdefghijklmopqrstuvwxyz', 512 / 2, 100, drystal.aligns.center)

	local text = 'abd {r:0|big|b:150|bla} {small|test} {big|%:50|defghi}'
	text = highlight(text, (math.sin(time/10)/2+0.5)*#text + 1)
	font_big:draw(text, 512 / 2, 512 / 2, drystal.aligns.center)

	font:draw_plain(text, 10, 512 * 0.7)
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

