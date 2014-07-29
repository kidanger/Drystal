local drystal = require 'drystal'

function drystal.init()
	drystal.resize(400, 400)
end

function drystal.update()
end

function drystal.draw()
end

local text = ''
local texting = false

function drystal.key_press(key)
	print('key_press', key)
	if texting then
		if key == 'return' then
			texting = false
			print('=>', text)
			drystal.stop_text()
		end
	else
		if key == 'i' then
			text = ''
			texting = true
			drystal.start_text()
		elseif key == 'a' then
			drystal.stop()
		end
	end
end
function drystal.key_release(key)
	print('key_release', key)
end

function drystal.key_text(str)
	print('key_text', str)
	text = text .. str
	print("text =", text)
end

