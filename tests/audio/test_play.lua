local drystal = drystal

piou = drystal.load_sound("test.wav")

function init()
	print("press p to play a wav sound :)")
	print("press 2 to play two times a wav sound with less volume :)")
	print("press * to play indefinetly a wav sound :)")
	print("press q to queue an Ogg music :)")
	print("press i to play indefinetly a Ogg music :)")
	print("press n to play directly a mp3 music :)")
	print("press l to lower the music volume :)")
	print("press z to lower the sound volume :)")
	drystal.resize(40, 40)
	drystal.set_resizable(true)
end

function key_press(key)
	if key == 'escape' then
		drystal.engine_stop()
	elseif key == 'p' then
		drystal.play_sound(piou)
	elseif key == 'l' then
		drystal.set_music_volume(0.5)
	elseif key == 'z' then
		drystal.set_sound_volume(0.5)
	elseif key == 'q' then
		drystal.play_music_queued("test.ogg")
	elseif key == 'n' then
		drystal.play_music("test.mp3")
	elseif key == 'i' then
		drystal.play_music("test.ogg", -1)
	elseif key == '[2]' then
		drystal.play_sound(piou, 2, 0.2)
	elseif key == '*' then
		drystal.play_sound(piou, -1)
	end
end
