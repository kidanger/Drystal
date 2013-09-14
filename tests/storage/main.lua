local drystal = drystal
local storage = require 'storage'

local data = storage.load('test_storage')
print(data)

print('saving...')
storage.save('test_storage', {text='blabla'})
print('saved')

data = storage.load('test_storage')

if not data then
	print('data not loaded')
else
	print(data.text)
end

function drystal.init()
	drystal.engine_stop()
end
