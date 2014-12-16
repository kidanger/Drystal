local drystal = require 'drystal'

local piou = assert(drystal.load_sound("test.wav"))

function drystal.init()
	print("press p to play a wav sound")
	print("press i to play a ogg music")
	print("press o to pause a ogg music")
	print("press l to lower the music volume")
	print("press z to lower the sound volume")
	drystal.resize(40, 40)
end

local music = assert(drystal.load_music("test.ogg"))

local pitch = 1.0
function drystal.key_press(key)
	if key == 'a' then
		drystal.stop()
	elseif key == 'p' then
		piou:play(1, 0, 0, pitch)
		print('play sound')
	elseif key == '[+]' then
		pitch = pitch + 0.10
		music:set_pitch(pitch)
	elseif key == '[-]' then
		pitch = pitch - 0.10
		music:set_pitch(pitch)
	elseif key == 'l' then
		drystal.set_music_volume(0.3)
	elseif key == 'z' then
		drystal.set_sound_volume(0.3)
	elseif key == 'i' then
		music:play()
		print('play music')
	elseif key == 'o' then
		music:pause()
		print('pause music')
	elseif key == 'o' then
		music:stop()
		print('stop music')
	end
end
