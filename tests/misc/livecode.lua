local l = require 'livecode2'

function drystal.init()
	print('============')
	if not drystal.screen then
		drystal.resize(math.random(100, 300), math.random(100, 300))
	end
	print('init', l.value)
	print('send SIGUSR1 to drystal or press \'x\'')
end

function drystal.update()
end

function drystal.key_press(k)
	if k == 'x' then
		drystal.reload()
	end
end

