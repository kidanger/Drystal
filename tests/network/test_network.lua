net_status = {
	hostname = 'localhost',
	port = '1234',
	status = '',
	last_received = '',
}

function init()
	print("initialized from lua")
	resize(40, 40)
	set_resizable(true)

	do_connect()
end

function key_press(key)
	if key == 'escape' then
		engine_stop()
	end
end

function key_release(key)
end

function do_connect()
	net_status.info = 'connecting'
	local ok = connect(net_status.hostname, net_status.port)
	if ok == 0 then
		net_status.info = 'connection failed'
	end
	send('test test test')
end

function receive(str)
	print('-> ' .. str)
end

function connected()
	print('Connected.')
	net_status.info = 'connected'
end
function disconnected()
	print('Disconnected from the server.')
	net_status.info = 'disconnected'
end
