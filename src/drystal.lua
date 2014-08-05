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
