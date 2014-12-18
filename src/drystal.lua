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

