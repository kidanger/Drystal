local drystal = require 'drystal'

--[[
	From https://github.com/rxi/lume/
]]
local lume = {}
function lume.clone(t)
	local rtn = {}
	for k, v in pairs(t) do rtn[k] = v end
	return rtn
end
local patternescape = function(str)
	return str:gsub("[%(%)%.%%%+%-%*%?%[%]%^%$]", "%%%1")
end
function lume.trim(str, chars)
	if not chars then return str:match("^[%s]*(.-)[%s]*$") end
	chars = patternescape(chars)
	return str:match("^[" .. chars .. "]*(.-)[" .. chars .. "]*$")
end
function lume.hotswap(modname)
	local oldglobal = lume.clone(_G)
	local updated = {}
	local function update(old, new)
		if not old or updated[old] then return end
		updated[old] = true
		local oldmt, newmt = getmetatable(old), getmetatable(new)
		if oldmt and newmt then update(oldmt, newmt) end
		for k, v in pairs(new) do
			if type(v) == "table" then update(old[k], v) else old[k] = v end
		end
	end
	local err = nil
	local function onerror(e)
		for k, v in pairs(_G) do _G[k] = oldglobal[k] end
		err = lume.trim(e)
	end
	local ok, oldmod = pcall(require, modname)
	oldmod = ok and oldmod or nil
	xpcall(function()
		package.loaded[modname] = nil
		local newmod = require(modname)
		if type(oldmod) == "table" then update(oldmod, newmod) end
		for k, v in pairs(oldglobal) do
			if v ~= _G[k] and type(v) == "table" then
				update(v, _G[k])
				_G[k] = v
			end
		end
	end, onerror)
	package.loaded[modname] = oldmod
	if err then return nil, err end
	return oldmod
end

local errors = {}

function drystal.hotswap(filename)
	if not filename:match('%.lua$') then
		return
	end
	local luafile = filename:gsub('%.lua$', ''):gsub('[/\\\\]', '.'):gsub('^%.*', '')
	if not luafile then
		return
	end

	local init = drystal.init
	errors[filename] = true
	local ok, err = lume.hotswap(luafile)
	if err then
		error(err)
	end
	errors[filename] = nil
	if drystal.init ~= init and drystal.init then
		drystal.init()
	end
	return true
end

function drystal._reload_files(files)
	for _, file in ipairs(files) do
		if not drystal.file_exists(file) then
			goto continue
		end
		if drystal.hotswap(file) then
			print('Hotswapped: ' .. file)
		elseif drystal.reload_sound and drystal.reload_sound(file) then
		elseif drystal.reload_surface and drystal.reload_surface(file) then
		end
		::continue::
	end

	-- run if there are no errors
	return not next(errors)
end

