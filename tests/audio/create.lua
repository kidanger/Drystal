local drystal = require 'drystal'

local w = 800
local h = 600

local sound
local x = 0
local y = 0

function play(time, ...)
	if sound then
		drystal.free_sound(sound)
	end

	local freqs = {...}
	print("gen...")

	local sound = drystal.create_sound(function(i)
		local s = 0
		for _, f in ipairs(freqs) do
			s = s + math.sin(i*f*math.pi / 44100)
		end
		s = s / #freqs
		return s
	end, time*44100)

	drystal.play_sound(sound, 0.3, (x-w/2)/w, (y-h/2)/h)
	print("play at", ...)
end

function drystal.init()
	drystal.resize(w, h)
end

local time = 0
function drystal.update(dt)
	dt = dt / 1000
	time = time + dt

	if time > .2 then
		local freq = 1800 * x / w
		local freq2 = 1800 * y / h
		play(time+dt, freq, freq2)
		time = 0
	end
end

function drystal.draw(dt)
end

function drystal.mouse_motion(xx, yy)
	x = xx
	y = yy
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

