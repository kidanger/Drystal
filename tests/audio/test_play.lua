piou = load_sound("tests/audio/test.wav")
function init()
	print("press p to play a wav sound :)")
	print("press 2 to play two times a wav sound :)")
	print("press * to play indefinetly a wav sound :)")
	print("press q to queue an Ogg music :)")
	print("press i to play indefinetly a Ogg music :)")
	print("press n to play directly a mp3 music :)")
	resize(40, 40)
	set_resizable(true)
end

function key_press(key)
	if key == 'escape' then
		engine_stop()
	elseif key == 'p' then
		play_sound(piou)
	elseif key == 'q' then
		play_music_queued("tests/audio/test.ogg")
	elseif key == 'n' then
		play_music("tests/audio/test.mp3")
	elseif key == 'i' then
		play_music("tests/audio/test.ogg", -1)
	elseif key == '[2]' then
		play_sound(piou, 2)
	elseif key == '*' then
		play_sound(piou, -1)
	end
end
