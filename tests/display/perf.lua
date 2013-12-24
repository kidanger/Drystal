local drystal = require 'drystal'

local W, H = 600, 400
local target = 16
local number = 300

function drystal.init()
	drystal.resize(W, H)
end

local tick = 0
function drystal.update(dt)
	if tick > 100 then
		if dt > target then
			number = number - 5
		else
			number = number + 30
		end
	end
	tick = tick + 1
	if tick % 60 == 0 then
		print(number, dt - target)
	end
end

function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	drystal.set_alpha(255)
	drystal.set_color(255, 0, 0)
	local random = math.random
	for i = 1, number do
		local x = random(W)
		local y = random(H)
		local w = random(20)
		local h = random(20)
		local w2 = random(20)
		local h2 = random(20)
		drystal.draw_triangle(x, y, x+w, y+h, x+w2, y+h2)
	end
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end
