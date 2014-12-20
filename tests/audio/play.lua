local drystal = require 'drystal'

local piou = assert(drystal.load_sound("test.wav"))

function drystal.init()
	print("press p to play a wav sound")
	print("press i to play a ogg music")
	print("press o to pause a ogg music")
	print("press l to lower the volume of every music")
	print("press z to lower the volume of every sound")
	print("press t to set the music volume to 0.5")
	print("press - to decrease the pitch")
	print("press + to increase the pitch")
	drystal.resize(40, 40)
end

local music = assert(drystal.load_music("test.ogg"))

local pitch = 1.0
local sound_volume = 1.0
local music_volume = 1.0
function drystal.key_press(key)
	if key == 'a' then
		drystal.stop()
	elseif key == 'p' then
		piou:play(1, 0, 0, pitch)
		print('play sound')
	elseif key == '[+]' then
		pitch = pitch + 0.10
		music:set_pitch(pitch)
		print('pitch: ', pitch)
	elseif key == '[-]' then
		pitch = pitch - 0.10
		music:set_pitch(pitch)
		print('pitch: ', pitch)
	elseif key == 't' then
		music:set_volume(0.5)
	elseif key == 'l' then
		music_volume = music_volume - 0.3
		drystal.set_music_volume(music_volume)
	elseif key == 'z' then
		sound_volume = sound_volume - 0.3
		drystal.set_sound_volume(sound_volume)
	elseif key == 'i' then
		music:play(false, function()
			print "music ended"
		end)
		print('play music')
	elseif key == 'o' then
		music:pause()
		print('pause music')
	elseif key == 'o' then
		music:stop()
		print('stop music')
	end
end
