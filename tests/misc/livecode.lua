local l = require 'livecode2'

print('value:', l.value)

function drystal.init()
	print('============')
	if not drystal.screen then
		drystal.resize(math.random(100, 300), math.random(100, 300))
	end
	print('send SIGUSR1 to drystal or press \'x\'')
end

function drystal.update()
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	elseif k == 'x' then
		drystal.reload()
	end
end

