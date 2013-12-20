local drystal = drystal

function drystal.init()
	print'init'
	drystal.resize(400, 400)
end

function drystal.update()
	print'update'
end

function drystal.draw()
	print'draw'
end

function drystal.key_press(key, unicode)
	print('key_press', key, unicode)
	if key == 'a' then
		drystal.engine_stop()
	end
end
function drystal.key_release(key)
	print('key_release', key)
end

function drystal.mouse_press(x, y, b)
	print('mouse_press', x, y, b)
end
function drystal.mouse_motion(x, y, dx, dy)
	print('mouse_motion', x, y, dx, dy)
end
function drystal.mouse_release(x, y, b)
	print('mouse_release', x, y, b)
end

function drystal.atexit()
	print'atexit'
end

