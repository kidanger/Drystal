local drystal = drystal
local web = require 'web'

function drystal.init()
	drystal.resize(400, 400)
	print 'Press h to hide, g to grab'
end

local grabbed = false
local hidden = false
function drystal.key_press(key)
	if key == 'a' then
		drystal.stop()
	elseif key == 'h' then
		hidden = not hidden
		drystal.show_cursor(not hidden)
	elseif key == 'g' then
		grabbed = not grabbed
		drystal.grab_cursor(grabbed)
	end
	print('grab:', grabbed, 'hide:', hidden)
end
