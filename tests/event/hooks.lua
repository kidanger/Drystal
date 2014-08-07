local drystal = require 'drystal'

function drystal.init()
	print'init'
	drystal.resize(400, 400)
end

function drystal.update(dt)
	--print('update, dt')
end

function drystal.draw()
	--print'draw'
end

function drystal.key_press(key)
	print('key_press', key)
	if key == 'a' then
		drystal.stop()
	end
end
function drystal.key_release(key)
	print('key_release', key)
end
function drystal.key_text(char)
	print('key_text', char)
end

function drystal.mouse_press(x, y, b)
	if b == drystal.BUTTON_LEFT then
		print('button left pressed')
	end
	print('mouse_press', x, y, b)
end
function drystal.mouse_motion(x, y, dx, dy)
	print('mouse_motion', x, y, dx, dy)
end
function drystal.mouse_release(x, y, b)
	if b == drystal.BUTTON_LEFT then
		print('button left released')
	end
	print('mouse_release', x, y, b)
end

function drystal.atexit()
	print'atexit'
end

