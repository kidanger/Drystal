local _path = package.path
package.path = ''

local net = require 'net'
package.path = _path

local Socket = net.__Socket

net.all = {}

net.rawconnect = net.connect
net.rawaccept = net.accept

Socket.rawsend = Socket.send
Socket.rawrecv = Socket.recv
Socket.rawdisconnect = Socket.disconnect
Socket.rawflush = Socket.flush


function Socket:init()
	self.tosend = {}
	self.buffer = ''
	self.autoflush = false
	self.has_errors = false
end


function Socket:send(data)
	if self.autoflush then
		self:flush()
		if not self.has_errors then
			local _, err = self:rawsend(data)
			if err then
				print('send', err)
				self.has_errors = true
				return nil, err
			end
		end
	else
		table.insert(self.tosend, data)
	end
end

function Socket:flush()
	if self.tosend[1] then
		local data = table.concat(self.tosend)
		local _, err = self:rawsend(data)
		if err then
			print('flush', err)
			self.has_errors = true
			return nil, err
		end
		self.tosend = {}
	end
	local _, err = self:rawflush() -- some data might be stuck in the C++ buffer
	if err then
		print('rawflush', err)
		self.has_errors = true
		return nil, err
	end
end

function Socket:recv()
	local str, err = self:rawrecv()
	if err then
		print('recv', err)
		self.has_errors = true
		return nil, err
	end
	return str
end


function Socket:sendline(line)
	self:send(line .. '\n')
end

function Socket:recvline()
	local str, err = self:recv()
	if err then
		return nil, err
	end

	if str then
		self.buffer = self.buffer .. str
	end
	if not self.buffer then
		return nil
	end

	local line = string.match(self.buffer, "[^\n]+\n")
	if line then
		self.buffer = self.buffer:sub(#line + 1)
		line = line:sub(1, -2)
		return line
	end
	return nil
end


local function convert_lua_to_data(lua)
	local serialized = drystal.serialize(lua)
	local netstring = table.concat({#serialized, ':', serialized})
	return netstring
end

function Socket:sendlua(lua)
	local data = convert_lua_to_data(lua)
	self:send(data)
end

function Socket:recvlua()
	local str, err = self:recv()
	if err then
		return nil, err
	end

	if str then
		self.buffer = self.buffer .. str
	end
	if not self.buffer then
		return nil
	end

	if not self.current_size then
		-- try to get the size of the data from the buffer
		local _, stop = self.buffer:find('(%d+):')
		if stop then
			self.current_size = tonumber(self.buffer:sub(1, stop - 1))
			self.buffer = self.buffer:sub(stop + 1)
		end
	elseif self.current_size and #self.buffer >= self.current_size then
		-- we know the current message length,
		-- and the buffer is longer than that : EXTRACT
		local data = self.buffer:sub(1, self.current_size)
		self.buffer = self.buffer:sub(self.current_size + 1)
		self.current_size = nil
		return drystal.deserialize(data)
	end

	return nil
end

function Socket:disconnect()
	local all = net.all
	for i, s in ipairs(all) do
		if s == self then
			all[i] = all[#all]
			all[#all] = nil
			break
		end
	end
	self:rawdisconnect()
end

function Socket:set_debug()
	local oldsend = self.send
	local oldrecv = self.recv
	self.send = function(c, msg)
		print('<', msg)
		return oldsend(c, msg)
	end
	self.recv = function(c)
		local str = oldrecv(c)
		if str then
			print('>', str)
		end
		return str
	end
end

function net.connect(...)
	local socket = net.rawconnect(...)
	if socket then
		socket:init()
		socket.autoflush = true
	end
	return socket
end

function net.accept(cb, ...)
	return net.rawaccept(function (socket)
		if socket then
			socket:init()
		end
		if cb(socket) ~= false then
			table.insert(net.all, socket)
			return true
		end
		socket:disconnect()
		return false
	end, ...)
end


function net.generic_send_all(method, message, sockets, except)
	for _, s in ipairs(sockets or net.all) do
		if s ~= except then
			s[method](s, message)
		end
	end
end

function net.generic_recv_all(method, callback, sockets)
	for _, s in ipairs(sockets or net.all) do
		repeat
			local msg = s[method](s)
			if msg then
				callback(s, msg)
			end
		until not msg
	end
end

function net.send_all(...)
	return net.generic_send_all('send', ...)
end
function net.sendline_all(...)
	return net.generic_send_all('sendline', ...)
end
function net.sendlua_all(message, ...)
	local data = convert_lua_to_data(message) -- cache the serialized lua
	return net.generic_send_all('send', data, ...)
end

function net.recv_all(...)
	return net.generic_recv_all('recv', ...)
end
function net.recvline_all(...)
	return net.generic_recv_all('recvline', ...)
end
function net.recvlua_all(...)
	return net.generic_recv_all('recvlua', ...)
end

function net.flush_all(sockets)
	for _, s in ipairs(sockets or net.all) do
		s:flush()
	end
end

function net.drop_clients(on_drop, sockets)
	local i = 1
	sockets = sockets or net.all
	while sockets[i] do
		local s = sockets[i]
		if s.has_errors then
			s:disconnect()
			on_drop(s)
			if sockets ~= net.all then
				-- otherwise, we miss some sockets
				i = i + 1
			end
		else
			i = i + 1
		end
	end
end

