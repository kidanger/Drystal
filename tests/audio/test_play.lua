function init()
	print("press p for direct music play :)")
	print("press q for queued music play :)")
	resize(40, 40)
	set_resizable(true)
end

function key_press(key)
	if key == 'escape' then
		engine_stop()
	end
	if key == 'p' then
		play_sound("tests/audio/test.wav")
	end
	if key == 'q' then
		play_sound_queued("tests/audio/test.wav")
	end
end
