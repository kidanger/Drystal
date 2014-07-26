local drystal = require "drystal"

local arial
function drystal.init()
	drystal.resize(600, 400);
	arial = assert(drystal.load_font('arial.ttf', 25))
end
local time = 0

function drystal.update(dt)
	time = time + dt
end

local shadowx = 0
local shadowy = 0
function drystal.draw()
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	drystal.set_color(0, 0, 0)
	local text = '{outline|outg:%d|yeah it\'s c{outg:0|o}ol!}'
	text = text:format((math.sin(time) / 2 + .5)*255)
	local y = 20
	local x = 20

	local w, h = arial:sizeof_plain(text)
	drystal.draw_square(x, y, w, h)
	arial:draw_plain(text, x, y)

	y = y + h + 5
	local w, h = arial:sizeof(text)
	drystal.draw_square(x, y, w, h)
	drystal.set_color(255, 255, 255)
	arial:draw(text, x, y)

	drystal.set_color(0, 0, 0)
	local shadowed = 'Oh {shadowx:%.2f|shadowy:%.2f|outline|r:200|waw!}'
	shadowed = shadowed:format(shadowx, shadowy)
	arial:draw(shadowed, 300, 200, 2)
end

function drystal.mouse_motion(x, y)
	shadowx = (300 - x) / 300 * 5
	shadowy = (200 - y) / 200 * 5
end
