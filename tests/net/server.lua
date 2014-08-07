local drystal = require 'drystal'
require 'common'

drystal.listen(port)
print('listening on', port)

local client
while not client do
	drystal.accept(function(c)
		print(c.address, 'connected')
		client = c
		c:set_debug()
	end, 1)

	if not client then
		print 'no client found'
	end
end

drystal.flush_all()

for i = 1, 5 do
	waittoken(client, 'hello')
end

client:sendline(1321)

function drystal.update(dt)
	local x = client:recvline()
	if x then
		client:sendline(x + 1)
	end

	drystal.flush_all()
	drystal.drop_clients(function(c) print(c, 'disconnected') end)
end

