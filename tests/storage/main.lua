local drystal = drystal

local data = drystal.fetch('test_storage')
print(data)
if data then
	print(data.text)
end

print('saving...')
drystal.store('test_storage', {text='blabla'})
print('saved')

data = drystal.fetch('test_storage')

if not data then
	print('data not loaded')
else
	print(data.text)
end

function drystal.init()
	drystal.stop()
end
