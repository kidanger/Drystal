local drystal = require 'drystal'

local s = drystal.screen

s.x = 4
assert(s.x == 4)
assert(s.draw_on ~= nil)
assert(getmetatable(s).draw_on ~= nil)
assert(s.__self == s)

assert(getmetatable(getmetatable(s)) == drystal.Surface)
assert(getmetatable(getmetatable(s)).draw_on ~= nil)

for k, v in pairs(getmetatable(s)) do
	print(k, v)
end

drystal.stop()

