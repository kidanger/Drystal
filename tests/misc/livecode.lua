local drystal = require 'drystal'
local l = require 'dir.livecode2'

drystal.resize(300, 200)

print('value:', l.value)

local old
local time = 0
function drystal.update(dt)
	time = time + dt
	if old ~= l.value then
		print(old, '->', l.value)
		old = l.value
	end
	if time > 1 then
		time = time - 1
		print('ping', dt)
	end
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	elseif k == 'x' then
		drystal.reload()
	end
end

