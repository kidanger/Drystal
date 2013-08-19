require 'drystal'
local web = require 'web'

--local audio = require 'audio'
--print(audio.init())

local sounds = {
	'jump1.wav',
	'jump2.wav',
	'hurt1.wav',
	'coin1.wav',
	'coin2.wav',
}

local loaded = {}
for i, s in pairs(sounds) do
	if not file_exists(s) then
		web.wget('gamedata/' .. s, s, function ()
			sound = load_sound(s)
			loaded[i] = sound
		end, function() print('can\'t download sound', s) end)
	else
		sound = load_sound(s)
		print(sound)
		loaded[i] = sound
	end
end

function init()
	resize(400, 400)
end

function update()
end

function draw()
end

function mouse_motion()
	play_sound(loaded[math.random(#loaded)])
end

function key_press(k)
	if k == 'escape' or k == 'a' then
		engine_stop()
	end
end
