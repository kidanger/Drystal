local drystal = require "drystal"
local font = require "truetype"

local arial
function drystal.init()
	drystal.resize(600, 400);
	arial = assert(font.load('arial.ttf', 25))
	font.use(arial)
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

	font.use_color(false)
	local w, h = font.sizeof(text)
	drystal.draw_square(x, y, w, h)
	font.draw(text, x, y)

	y = y + h + 5
	font.use_color(true)
	local w, h = font.sizeof(text)
	drystal.draw_square(x, y, w, h)
	drystal.set_color(255, 255, 255)
	font.draw(text, x, y)

	font.use_color(true)
	drystal.set_color(0, 0, 0)
	local shadowed = 'Oh {shadowx:%.2f|shadowy:%.2f|outline|r:200|waw!}'
	shadowed = shadowed:format(shadowx, shadowy)
	font.draw_align(shadowed, 300, 200, 'center')
end

function drystal.mouse_motion(x, y)
	shadowx = (300 - x) / 300 * 5
	shadowy = (200 - y) / 200 * 5
end
