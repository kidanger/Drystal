local drystal = require 'drystal'

local W, H = 600, 400
local target = 16
local number = 100000
local tick = 0

function drystal.init()
	drystal.resize(W, H)
end

local function compute(n)
	local acc = 1
	for i = 1, n do
		acc = acc * 1.45 + 1/acc
	end
	return acc
end


function drystal.update(dt)
	compute(number)
	if tick > 4 then
		if dt > target then
			number = number * .99
		else
			number = number * 1.02
		end
	end
	tick = tick + 1
	if tick % 60 == 0 then
		print(math.floor(number), dt - target)
	end
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

