local drystal = require 'drystal'

function drystal.init()
	drystal.resize(400, 300)
end

local time = 0

function drystal.update(dt)
	time = time + dt
	drystal.set_title(time)
end
