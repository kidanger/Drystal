local drystal = require 'drystal'
local l = require 'livecode2'

drystal.resize(300, 200)

print('value:', l.value)

function drystal.init()
	print('init')
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	elseif k == 'x' then
		drystal.reload()
	end
end

