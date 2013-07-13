piou = load_sound("tests/audio/test.wav")
function init()
	print("press p to play a wav sound :)")
	print("press q to queue an Ogg music :)")
	print("press n to play directly a mp3 music :)")
	resize(40, 40)
	set_resizable(true)
end

function key_press(key)
	if key == 'escape' then
		engine_stop()
	end
	if key == 'p' then
		play_sound(piou)
	end
	if key == 'q' then
		play_music_queued("tests/audio/test.ogg")
	end
	if key == 'n' then
		play_music("tests/audio/test.mp3")
	end
end
