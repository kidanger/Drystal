-- [[
-- Based on https://github.com/ignacio/StackTracePlus
-- ]]

assert(debug, "debug table must be available at this point")

local C_HIGHLIGHT_RED      = "\x1B[31m"
local C_BACKGROUND_RED     = "\x1B[41m"
local C_HIGHLIGHT_YELLOW   = "\x1B[33m"
local C_HIGHLIGHT_BLUE     = "\x1B[34m"
local C_HIGHLIGHT_GRAY     = "\x1B[90m"
local C_HIGHLIGHT_PURPLE   = "\x1B[35m"
local C_HIGHLIGHT_GREEN    = "\x1B[92m"
local C_RESET              = "\x1B[0m"
local C_BACKGROUND_RESET   = "\x1B[49m"
local C_ITALIC             = "\x1B[3m"
local C_UNDERLINED         = "\x1B[4m"

local function C(str, c)
	if str then
		return c .. str .. C_RESET
	end
	return ''
end

local function remove_colors(str)
	return str:gsub('(\x1B%[.-m)', '')
end

local m_known_tables = { [_G] = "_G" }
local m_user_known_tables = {}
local m_known_functions = {}
local m_user_known_functions = {}

local function add_known_module(name, desc)
	local ok, mod = pcall(require, name)
	if ok then
		m_known_tables[mod] = desc
	end
end

add_known_module("string", "string module")
add_known_module("io", "io module")
add_known_module("os", "os module")
add_known_module("table", "table module")
add_known_module("math", "math module")
add_known_module("package", "package module")
add_known_module("debug", "debug module")
add_known_module("coroutine", "coroutine module")
add_known_module("bit32", "bit32 module")
add_known_module("utf8", "utf8 module")

add_known_module("drystal", "drystal module")

for name, value in pairs(_G) do
	if type(value) == 'function' then
		m_known_functions[value] = name
	end
end


local function safe_tostring(value)
	local ok, err = pcall(tostring, value)
	if ok then return err else return ("<failed to get printable value>: '%s'"):format(err) end
end

local function parse_line(line)
	local expressions = {
		"^%s*function%s+(.+)%s*%(",
		"^%s*local%s+function%s+(%w+)",
		"^%s*local%s+(%w+)%s+=%s+function",
	}
	for _, expr in ipairs(expressions) do
		local match = line:match(expr)
		if match then
			return match
		end
	end
end

local function get_line(info, lineno)
	if not lineno then
		return
	end

	local source
	if type(info.source) == "string" and info.source:sub(1, 1) == "@" then
		local file = io.open(info.source:sub(2), "r")
		if not file then
			return
		end
		source = file:read("*a")
	else
		source = info.source
	end

	local l
	local iterator = source:gmatch("([^\n]*)\n")
	for i=1, lineno do
		l = iterator(l)
	end
	return l
end

local function guess_function_name(info)
	local line = get_line(info, info.linedefined)
	return line and parse_line(line)
end

local Dumper = {}
Dumper.__index = Dumper

Dumper.new = function(thread)
	local t = { lines = {} }
	setmetatable(t, Dumper)

	t.dumping_same_thread = thread == coroutine.running()

	if type(thread) == "thread" then
		t.getinfo = function(level, what)
			if t.dumping_same_thread and type(level) == "number" then
				level = level + 1
			end
			return debug.getinfo(thread, level, what)
		end
		t.getlocal = function(level, loc)
			if t.dumping_same_thread then
				level = level + 1
			end
			return debug.getlocal(thread, level, loc)
		end
		t.getupvalue = function(level, loc)
			if t.dumping_same_thread then
				level = level + 1
			end
			return debug.getupvalue(thread, level, loc)
		end
	else
		t.getinfo = debug.getinfo
		t.getlocal = debug.getlocal
		t.getupvalue = debug.getupvalue
	end

	return t
end

function Dumper:add(text)
	self.lines[#self.lines + 1] = text
end

local function show_variable(self, name, value)
	if name == '(*temporary)' then
		return
	end

	local valuestr
	if type(value) == "number" then
		valuestr = ('%g'):format(value)
	elseif type(value) == "boolean" then
		valuestr = tostring(value)
	elseif type(value) == "string" then
		valuestr = ('%q'):format(value)
	elseif type(value) == "userdata" then
		valuestr = safe_tostring(value)
	elseif type(value) == "nil" then
		valuestr = 'nil'
	elseif type(value) == "table" then
		if m_known_tables[value] then
			valuestr = m_known_tables[value]
		elseif m_user_known_tables[value] then
			valuestr = m_user_known_tables[value]
		else
			local txt = "{"
			local n_more = 0
			for k,v in pairs(value) do
				if #txt > 20 then
					n_more = n_more + 1
				else
					txt = txt..safe_tostring(k).."="..safe_tostring(v)
					if next(value, k) then txt = txt..", " end
				end
			end
			if n_more > 0 then
				txt = txt .. ('%d more'):format(n_more)
			end
			txt = txt .. '}'
			valuestr = safe_tostring(value) .. ' ' .. txt
		end
	elseif type(value) == "function" then
		local info = self.getinfo(value, "nS")
		local fun_name = info.name or m_known_functions[value] or m_user_known_functions[value]
		if info.what == "C" then
			valuestr = 'C' .. (fun_name and ("function: " .. fun_name) or tostring(value))
		else
			local source = info.short_src
			if source:sub(2,7) == "string" then
				source = 'buffer ' .. source:sub(10, source:find('"]') - 1)
			end
			fun_name = fun_name or guess_function_name(info)
			valuestr = ('function %q, %s:%d'):format(fun_name or '', source, info.linedefined)
		end
	elseif type(value) == "thread" then
		valuestr = 'thread ' .. tostring(value)
	end

	return valuestr
end

function Dumper:dump_locals(level, prefix)
	if self.dumping_same_thread then
		level = level + 1
	end

	local i = 1
	while true do
		local name, value = self.getlocal(level, i)
		if not name then break end

		local str = show_variable(self, name, value)
		if str then
			self:add(("%s%s = %s"):format(prefix, C(name, C_ITALIC .. C_HIGHLIGHT_GRAY), str))
		end

		i = i + 1
	end
end

function Dumper:dump_upvalues(func, prefix)
	local i = 1
	while true do
		local name, value = debug.getupvalue(func, i)
		if not name then break end

		local str = show_variable(self, name, value)
		if str then
			self:add(("%s%s = %s"):format(prefix, C('upvalue ' .. name, C_ITALIC .. C_HIGHLIGHT_GRAY), str))
		end

		i = i + 1
	end
end

local function stackinfo(info)
	local kind
	local filename
	local functionname
	local linenumber
	local locals

	if info.what == "main" then
		if info.source:sub(1, 1) == "@" then
			filename = info.source:sub(2)
			filename = filename:gsub('^%./', '')
		else
			filename = info.short_src
		end
		--kind = 'main chunk'
		linenumber = info.currentline
		locals = true

	elseif info.what == "C" then
		functionname = m_user_known_functions[info.func]
					or m_known_functions[info.func] or info.name or tostring(info.func)
		kind = (info.namewhat ~= '' and (info.namewhat .. ' ') or '') .. 'C function'

	elseif info.what == "Lua" then
		functionname = m_user_known_functions[info.func] or m_known_functions[info.func]
						or guess_function_name(info) or info.name

		if info.source and info.source:sub(1, 1) == "@" then
			filename = info.source:sub(2)
			filename = filename:gsub('^%./', '')
		else
			local source = info.short_src
			if source:sub(2, 7) == "string" then
				source = 'buffer ' .. source:sub(10, source:find('"]') - 1)
			end
			filename = source
		end
		if info.namewhat == 'method' then
			kind = 'method'
		else
			kind = 'function'
			if info.namewhat ~= '' then
				kind = info.namewhat .. ' ' .. kind
			end
		end
		linenumber = info.currentline
		locals = true
	else
		kind = 'unknown frame ' .. info.what
	end

	return ('%s%s%s%s%s'):format(
			C(kind or '', ''),
			functionname and (' ' .. C(functionname, C_HIGHLIGHT_BLUE)) or '',
			((kind or functionname) and filename) and ', ' or '',
			C(filename, C_UNDERLINED),
			C(linenumber and (':' .. linenumber), C_UNDERLINED)
		), locals
end

local function enhance_line(line, err)
	-- see lua/src/ldebug.c
	local kinds = { 'local', 'global', 'field', 'upvalue', 'constant', 'method' }
	local errvariable
	for _, k in ipairs(kinds) do
		errvariable = errvariable or err:match("attempt to .- a .- value %(" .. k .. " '(.-)'%)")
	end
	if errvariable then
		line = '-' .. line .. '-'
		print(line)
		while true do
			local old = line
			line = line:gsub('([^%w_])(' .. errvariable .. ')([^%w_])',
					'%1' .. C_BACKGROUND_RED .. '%2' .. C_BACKGROUND_RESET .. '%3')
			if old == line then break end
		end
		line = line:sub(2, -2)
		print(line)
	end
	return line
end

local function transform_message(message)
	if type(message) == "table" then
		local err = "{\n"
		for k, v in pairs(message) do
			err = err .. ('    %s=%s,\n'):format(safe_tostring(k), safe_tostring(v))
		end
		return err .. '}'
	elseif type(message) == "string" then
		return message:gsub('(.*:.*: )', '')
	elseif message then
		return tostring(message)
	end
end

local function stacktrace(thread, message, level, use_color)
	if type(thread) ~= "thread" then
		-- shift parameters left
		thread, message, level, use_color = nil, thread, message, level
	end

	thread = thread or coroutine.running()
	level = level or 1

	local dumper = Dumper.new(thread)
	local err = transform_message(message)
	
	dumper:add 'stack traceback:'
	
	local level_to_show = level - 1
	if dumper.dumping_same_thread then level = level + 1 end
	local infos = {}

	local info = dumper.getinfo(level, "nSlft")
	while info do
		if level >= 3 or (info.what ~= 'C' and not info.short_src:match('^%[string')) then
			table.insert(infos, {info=info, level=level})
		end

		level = level + 1
		level_to_show = level_to_show + 1
		info = dumper.getinfo(level, "nSlft")
	end

	for i=#infos, 1, -1 do
		local info = infos[i].info
		local level = infos[i].level
		local infostr, locals = stackinfo(info)
		dumper:add(('#%d in %s'):format(#infos - i + 1, infostr))
		local prefix = "   |   "
		if info.istailcall then
			dumper:add(prefix .. '... tail calls ...')
		end
		if locals and i <= 2 then
			dumper:dump_upvalues(info.func, prefix)
			dumper:dump_locals(level, prefix)
		end
		if info.currentline then
			local line = get_line(info, info.currentline)
			if line then
				line = line:gsub('^%s+', '')
				if i == 1 and err then
					line = enhance_line(line, err)
				end
				dumper:add(('   \\-> %s'):format(C(line, C_HIGHLIGHT_GREEN)))
			end
		end
	end

	if err then
		dumper:add(C('error: ' .. err, C_HIGHLIGHT_RED))
	end

	local lines = table.concat(dumper.lines, '\n')
	if not use_color then
		lines = remove_colors(lines)
	end
	return lines
end

debug.newtraceback = stacktrace

