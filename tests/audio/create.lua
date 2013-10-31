local drystal = require 'drystal'

local w = 800
local h = 600

local len = 1.6 * 44100
local sound

function play(freq)
	local new = drystal.create_sound(function(i)
		local s = math.sin(i*freq*math.pi / 44100)
		return s
	end, len)
	drystal.play_sound(new)
	print("play at", freq, new)

	if sound then
		drystal.free_sound(sound)
	end
	sound = new
end

function drystal.init()
	drystal.resize(w, h)
end

local x = 0
local time = 0
function drystal.update(dt)
	time = time + dt/1000
	if time > 0.1 then
		time = 0

		local freq = 1800 * x / w
		play(freq)
	end
end

function drystal.draw(dt)
end

function drystal.mouse_motion(xx, yy)
	x = xx
end

