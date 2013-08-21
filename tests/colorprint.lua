--[[
	Usage:
		print = require 'colorprint'
	or
		lua colorprint.lua test_colors
	Feel free to modify, redistribute, or whatever you want without restriction.
]]
local _print = print

local esc = string.char(27, 91)
local colors = {
	[true] = {
		esc .. '31m',
		esc .. '32m',
		esc .. '33m',
		esc .. '34m',
		esc .. '35m',
	},
	[false] = {
		esc .. '91m',
		esc .. '92m',
		esc .. '93m',
		esc .. '94m',
		esc .. '95m',
	}
}
local color_reset = esc .. '0m'

local light = true
local function get_color(i)
	return colors[light][1 + i % #colors[light]]
end

local function print(...)
	local args = {...}

	local last_arg = 0
	for i in pairs(args) do last_arg = i end

	-- nothing to print
	if last_arg == 0 then _print() return end

	-- add color before each parameters
	for i = 1, last_arg do
		local color
		if args[i] == '---' then
			color = esc .. '90m'
		else
			color = get_color(i)
		end
		args[i] = color .. tostring(args[i])
	end

	-- restore normal color
	args[#args] = args[#args] .. color_reset

	_print(unpack(args))

	-- flip between light and dark colors
	light = not light
end

local function test()
	math.randomseed(os.time())
	local function randomtable()
		local table = {}
		for i = 1, math.random(8) do
			table[i] = math.random() * math.random(1000)
		end
		return table, unpack(table)
	end
	local tables = {}
	for i = 1, 5 do
		tables[i] = {randomtable()}
	end

	for i = 1, #tables do
		print(unpack(tables[i]))
	end

	print('---')

	for i = 1, #tables do
		_print(unpack(tables[i]))
	end
end

-- uncomment to test the module
local args = {...}
if args[1] == 'test_colors' then
	test()
end

if os.getenv('HOME') then
	return print
else
	return _print
end
