port = '5544'

function waittoken(sock, token)
	local table
	while not table do
		table = sock:recvlua()
	end
	assert(table.msg == token)
end
