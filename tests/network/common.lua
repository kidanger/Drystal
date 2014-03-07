port = '5544'

function waittoken(sock, token)
	local str
	while not str do
		str = sock:recv()
	end
	assert(str == token)
end
