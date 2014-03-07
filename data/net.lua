local _path = package.path
package.path = ''

local net = require 'net'
package.path = _path

local allsockets = {}

net.rawconnect = net.connect
net.rawaccept = net.accept

local function wrap(socket)
	socket.tosend = {}
	socket.buffer = ''
	socket.autoflush = false
	socket.has_errors = false

	socket.rawsend = socket.send
	socket.send = function(socket, data)
		if socket.autoflush then
			socket:flush()
			if not socket.has_errors then
				local _, err = socket:rawsend(data)
				if err then
					print('send', err)
					socket.has_errors = true
					return nil, err
				end
			end
		else
			table.insert(socket.tosend, data)
		end
	end
	socket.sendline = function(socket, line)
		socket:send(line .. '\n')
	end
	socket.sendlua = function(socket, data)
		socket:send(drystal.serialize(data))
	end

	socket.flush = function(socket)
		if socket.tosend[1] then
			local data = table.concat(socket.tosend)
			local _, err = socket:rawsend(data)
			if err then
				print('flush', err)
				socket.has_errors = true
				return nil, err
			end
			socket.tosend = {}
		end
	end

	socket.rawrecv = socket.recv
	socket.recv = function(socket)
		local str, err = socket:rawrecv()
		if err then
			print('recv', err)
			socket.has_errors = true
			return nil, err
		end
		if str then
			socket.buffer = socket.buffer .. str
		end
		return str
	end
	socket.recvline = function(socket)
		local str, err = socket:recv()
		if err then
			return nil, err
		end

		if not socket.buffer then
			return nil
		end

		local line = string.match(socket.buffer, "[^\n]+\n")
		if line then
			socket.buffer = socket.buffer:sub(#line + 1)
			line = line:sub(1, -2)
			return line
		end
		return nil
	end
	socket.recvlua = function(socket)
		local str, err = socket:recv()
		if err then
			return nil, err
		end

		if not socket.buffer then
			return nil
		end

		local data = string.match(socket.buffer, "({.-})")
		if data then
		print(data)
			socket.buffer = socket.buffer:sub(#data + 1)
			return drystal.deserialize(data)
		end
		return nil
	end

	socket.set_debug = function(socket)
		local oldsend = socket.send
		local oldrecv = socket.recv
		socket.send = function(c, msg)
			print('<', msg)
			return oldsend(c, msg)
		end
		socket.recv = function(c)
			local str = oldrecv(c)
			if str then
				print('>', str)
			end
			return str
		end
	end

	socket.old_disconnect = socket.disconnect
	socket.disconnect = function(socket)
		for i, s in ipairs(allsockets) do
			if s == socket then
				allsockets[i] = allsockets[#allsockets]
				allsockets[#allsockets] = nil
			end
		end
		socket:old_disconnect()
	end
end

function net.connect(...)
	local socket = net.rawconnect(...)
	if socket then
		wrap(socket)
		socket.autoflush = true
	end
	return socket
end

function net.accept(cb, ...)
	return net.rawaccept(function (socket)
		wrap(socket)
		if cb(socket) ~= false then
			table.insert(allsockets, socket)
			return true
		end
		socket:disconnect()
		return false
	end, ...)
end


function net.generic_send_all(method, message, sockets, except)
	for _, s in ipairs(sockets or allsockets) do
		if s ~= except then
			s[method](s, message)
		end
	end
end

function net.generic_recv_all(method, callback, sockets)
	for _, s in ipairs(sockets or allsockets) do
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
function net.sendlua_all(...)
	return net.generic_send_all('sendlua', ...)
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
	for _, s in ipairs(sockets or allsockets) do
		s:flush()
	end
end

function net.drop_clients(on_drop, sockets)
	local i = 1
	sockets = sockets or allsockets
	while sockets[i] do
		local s = sockets[i]
		if s.has_errors then
			s:disconnect()
			on_drop(s)
			if sockets ~= allsockets then
				-- otherwise, we miss some sockets
				i = i + 1
			end
		else
			i = i + 1
		end
	end
end

function net.get_all_sockets()
	return allsockets
end

