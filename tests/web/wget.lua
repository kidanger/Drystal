local drystal = require 'drystal'

local function onsuccess(filename)
	print(filename, 'wgetted!')
	print(io.open(filename):read('*all'))
end

local function onerror(filename)
	print(filename, 'ko')
end

function drystal.init()
	print(drystal.is_web)
	drystal.wget('/', 'f1', onsuccess, onerror)
	drystal.wget('src/log.hpp', 'f2', onsuccess, onerror)
	drystal.wget('http://google.fr', 'f3', onsuccess, onerror)
	print 'boom'
end

function drystal.update()
	drystal.stop()
end

