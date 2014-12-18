local drystal = require 'drystal'

function drystal.file_exists(name)
	local f = io.open(name, "r")
	if f then
		f:close(f)
		return true
	end
	return false
end

function table.print(t)
	local str = {}
	for k, v in pairs(t) do
		table.insert(str, ('%s=%s'):format(k, v))
	end
	print(table.concat(str, ','))
end

function math.distance(x1, y1, x2, y2)
	return math.sqrt(math.pow(x1 - x2, 2) + math.pow(y1 - y2, 2))
end

function math.clamp(v, min, max)
	return math.min(math.max(v, min), max)
end

function math.aabb(o1, o2)
	local x1 = o1.x
	local y1 = o1.y
	local x2 = o2.x
	local y2 = o2.y
	return x2 <= x1 + o1.w
	and x2 + o2.w >= x1
	and y2 <= y1 + o1.h
	and y2 + o2.h >= y1
end

function math.inside(o, x, y)
	local xo = o.x
	local yo = o.y
	return x >= xo and y >= yo
	and x < xo + o.w and y < yo + o.h
end

drystal.Timer = {
	time=0,
	update=function(self, dt)
		if self.finished then return end
		self.time = self.time + dt
		if self.time >= self.duration then
			if self.callback then
				self.callback()
			end
			self.finished = true
		end
	end,
}
drystal.Timer.__index = drystal.Timer
function drystal.new_timer(duration, callback)
	return setmetatable({
		duration=duration,
		callback=callback,
	}, drystal.Timer)
end

