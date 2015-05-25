local drystal = require 'drystal'
local System = drystal.System

local function is_getset(funcname)
	if funcname:sub(1, string.len('set_min_')) == 'set_min_' then
		return true
	end
	return false
end

local funcs = {}
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
		funcs['set_' .. attr] = set_both
		funcs['get_' .. attr] = get_both
	end
end

for k, v in pairs(funcs) do
	System[k] = v
end

local function preprocess(data)
	local s = {}
	for at, value in pairs(data) do
		table.insert(s, {at=at, value=value})
	end
	table.sort(s, function(a, b) return a.at < b.at end)
	if s[1].at ~= 0 then
		table.insert(s, 1, {at=0, value=s[1].value})
	end
	if s[#s].at ~= 1 then
		table.insert(s, {at=1, value=s[#s].value})
	end
	return s
end

function System:set_sizes(sizes)
	self:clear_sizes()
	local s = preprocess(sizes)
	for _, data in ipairs(s) do
		if type(data.value) == 'table' then
			self:add_size(data.at, unpack(data.value))
		else
			self:add_size(data.at, data.value)
		end
	end
end

function System:set_colors(colors)
	self:clear_colors()
	local c = preprocess(colors)
	for _, data in ipairs(c) do
		local c1, c2 = data.value, data.value
		if type(data.value[1]) == 'table' or type(data.value[1]) == 'string' then
			c1 = data.value[1]
			c2 = data.value[2]
		end
		if type(c1) == 'string' then
			c1 = drystal.colors[c1]
		end
		if type(c2) == 'string' then
			c2 = drystal.colors[c2]
		end
		self:add_color(data.at, c1[1], c2[1], c1[2], c2[2], c1[3], c1[3])
	end
end

function System:set_alphas(alphas)
	self:clear_alphas()
	local a = preprocess(alphas)
	for _, data in ipairs(a) do
		if type(data.value) == 'table' then
			self:add_alpha(data.at, unpack(data.value))
		else
			self:add_alpha(data.at, data.value)
		end
	end
end

