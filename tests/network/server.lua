local drystal = drystal
local net = require 'net'
require 'common'

net.listen(port)
print('listening on', port)

local client
while not client do
	net.accept(function(c)
		print(c.address, 'connected')
		client = c
		c:set_debug()
	end, 1)

	if not client then
		print 'no client found'
	end
end

net.flush_all()

for i = 1, 5 do
	waittoken(client, 'hello')
end

client:sendline(1321)

function drystal.update(dt)
	local x = client:recvline()
	if x then
		client:sendline(x + 1)
	end

	net.flush_all()
	net.drop_clients(function(c) print(c, 'disconnected') end)
end

