local drystal = require 'drystal'
math.randomseed(os.time())

local font = assert(drystal.load_font('arial.ttf', 20))

function gen()
	local text = ''
	local chars = "{}|r:9"
	for i = 1, math.random(30) do
		local r =math.random(#chars)
		text = text .. chars:sub(r, r)
	end
	return text
end

for i = 1, 100 do
	local text = gen()
	print(text, font:sizeof(text))
end

drystal.stop()

