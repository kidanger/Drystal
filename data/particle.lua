local _path = package.path
package.path = ''

local particle = require 'particle'
package.path = _path

particle.rawnew_system = particle.new_system

local System = {}
System.__index = System

local function goes_to_metatable(funcname)
	if funcname:sub(1, string.len('get_')) == 'get_'
	or funcname:sub(1, string.len('set_')) == 'set_'
	or funcname == 'free' or funcname == 'draw' or funcname == 'update'
	or funcname == 'start' or funcname == 'pause' or funcname == 'stop'
	or funcname == 'is_running' then
		return true
	end

	return false
end

for k, v in pairs(particle) do
	if goes_to_metatable(k) then
		System[k] = function(self, ...) return v(self.data, ...) end
		particle[k] = function(self, ...) return v(self.data, ...) end
	end
end

function particle.new_system(...)
	return setmetatable({data=particle.rawnew_system(...)}, System)
end

return particle

