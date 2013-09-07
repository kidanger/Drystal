local drystal = drystal
local web = require 'web'

function init()
	drystal.resize(400, 400)
end

local grabbed = false
local hidden = false
function key_press(key)
	if key == 'a' then
	elseif key == 'h' then
		hidden = not hidden
		drystal.show_cursor(not hidden)
	elseif key == 'g' then
		grabbed = not grabbed
		drystal.grab_cursor(grabbed)
	end
	print('grab:', grabbed, 'hide:', hidden)
end
