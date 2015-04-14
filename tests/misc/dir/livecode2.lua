local L = {
	value=1,
}
L.__index = L

function L.new()
	return setmetatable({}, L)
end

return L

