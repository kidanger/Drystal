local drystal = require 'drystal'
local System = drystal.System

local function is_getset(funcname)
	if funcname:sub(1, string.len('set_min_')) == 'set_min_' then
		return true
	end
	return false
end

for k, v in pairs(System) do
	if is_getset(k) then
		local attr = k:sub(string.len('get_min_') + 1)
		local getmin = System['get_min_' .. attr]
		local getmax = System['get_max_' .. attr]
		local setmin = System['set_min_' .. attr]
		local setmax = System['set_max_' .. attr]
		local get_both = function(data)
			return getmin(data), getmax(data)
		end
		local set_both = function(data, min, max)
			setmin(data, min)
			setmax(data, max or min)
		end
		System['set_' .. attr] = set_both
		System['get_' .. attr] = get_both
	end
end

