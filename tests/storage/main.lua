local drystal = require 'drystal'

drystal.store('test_storage', {text='blabla', duck=58})
local data = drystal.fetch('test_storage')
assert(data)
assert(data.text == 'blabla')
assert(data.duck == 58)
drystal.store('test_lol', {text='blablalol', duck=586})
data = drystal.fetch('test_storage')
assert(data)
assert(data.text == 'blabla')
assert(data.duck == 58)
data = drystal.fetch('test_lol')
assert(data)
assert(data.text == 'blablalol')
assert(data.duck == 586)
data = drystal.fetch('unknown')
assert(not data)

function drystal.init()
	drystal.stop()
end
