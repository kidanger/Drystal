local _path = package.path
package.path = ''

local particle = require 'particle'
package.path = _path

particle.rawnew_system = particle.new_system

local System = {}
System.__index = System

local function is_getset(funcname)
	if funcname:sub(1, string.len('set_min_')) == 'set_min_' then
		return true
	end
	return false
end
local function goes_to_metatable(funcname)
	if funcname:sub(1, string.len('get_')) == 'get_'
		or funcname:sub(1, string.len('set_')) == 'set_'
		or funcname == 'free' or funcname == 'draw' or funcname == 'update'
		or funcname == 'start' or funcname == 'pause' or funcname == 'stop'
		or funcname == 'is_running' or funcname == 'add_size'
		or funcname == 'add_color' then
		return true
	end

	return false
end

for k, v in pairs(particle) do
	if is_getset(k) then
		local attr = k:sub(string.len('get_min_') + 1)
		local getmin = particle['get_min_' .. attr]
		local getmax = particle['get_max_' .. attr]
		local setmin = particle['set_min_' .. attr]
		local setmax = particle['set_max_' .. attr]
		local get_both = function(data)
			return getmin(data), getmax(data)
		end
		local set_both = function(data, min, max)
			setmin(data, min)
			setmax(data, max or min)
		end
		particle['get_' .. attr] = get_both
		particle['set_' .. attr] = set_both
	end
end

for k, v in pairs(particle) do
	if goes_to_metatable(k) then
		local f = function(self, ...) return v(self.data, ...) end
		System[k] = f
		particle[k] = f
	end
end

function particle.new_system(...)
	return setmetatable({data=particle.rawnew_system(...)}, System)
end

return particle

