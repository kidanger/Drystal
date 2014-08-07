local drystal = require 'drystal'

function drystal.init()
	drystal.resize(400, 400)
	print 'Press h to hide, r to set relative mode'
end

local relative = false
local hidden = false
function drystal.key_press(key)
	if key == 'a' then
		drystal.stop()
	elseif key == 'h' then
		hidden = not hidden
		drystal.show_cursor(not hidden)
	elseif key == 'r' then
		relative = not relative
		drystal.set_relative_mode(relative)
	end
	print('relative:', relative, 'hide:', hidden)
end

function drystal.mouse_motion(x, y, dx, dy)
	print(x, y, dx, dy)
end
