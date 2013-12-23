local drystal = require 'drystal'

local w = 800
local h = 500
local x = w/2
local y = h/2
local RATE = 44100

local current = 0

function play(time, ...)
	local freqs = {...}
	print("gen...")
	local new = drystal.create_sound(function(i)
		local s = 0
		for _, f in ipairs(freqs) do
			s = s + math.sin(i*f*math.pi / RATE)
		end
		s = s / #freqs
		return s
	end, time*RATE)
	new:play()
	print("play at", ...)
end

local cursor = 0
function music_callback(data, len)
	local freq = 1800 * x / w
	local tone = freq * math.pi / RATE
	for i = 1, len do
		data[i] = math.sin(cursor * tone)
		cursor = cursor + 1
	end
	return len
end

function drystal.init()
	drystal.resize(w, h)
	drystal.set_music_volume(.2)
	local music = drystal.load_music(music_callback, RATE)
	music:play()
end

function drystal.update(dt)
	dt = dt / 1000
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

