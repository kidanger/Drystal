piou = load_sound("data/test.wav")
function init()
	print("press p to play a wav sound :)")
	print("press 2 to play two times a wav sound with less volume :)")
	print("press * to play indefinetly a wav sound :)")
	print("press q to queue an Ogg music :)")
	print("press i to play indefinetly a Ogg music :)")
	print("press n to play directly a mp3 music :)")
	print("press l to lower the music volume :)")
	print("press z to lower the sound volume :)")
	resize(40, 40)
	set_resizable(true)
end

function key_press(key)
	if key == 'escape' then
		engine_stop()
	elseif key == 'p' then
		play_sound(piou)
	elseif key == 'l' then
		set_music_volume(0.5)
	elseif key == 'z' then
		set_sound_volume(0.5)
	elseif key == 'q' then
		play_music_queued("data/test.ogg")
	elseif key == 'n' then
		play_music("data/test.mp3")
	elseif key == 'i' then
		play_music("data/test.ogg", -1)
	elseif key == '[2]' then
		play_sound(piou, 2, 0.2)
	elseif key == '*' then
		play_sound(piou, -1)
	end
end
