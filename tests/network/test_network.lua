local net = require 'net'

local hostname = 'localhost'
local port = '1234'

function init()
	resize(40, 40)

	local err = net.connect(hostname, port)
	if err ~= net.NO_ERROR then
		error('connection failed ' .. err)
	else
		print'connected'
	end
	print('send', net.send('test test test'))
end

function key_press(key)
	if key == 'escape' then
		net.disconnect();
		engine_stop()
	end
end

function update(dt)
	local code, str = net.receive()
	if code == 0 then
	elseif code > 0 then
		print('received:', str)
	else
		error('err receiving ' .. code)
	end
end

