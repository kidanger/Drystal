local drystal = require 'drystal'
local web = require 'web'

function drystal.init()
	print(web.is_web())
	web.wget('/', 'f1', onsuccess, onerror)
	web.wget('src/log.hpp', 'f2', onsuccess, onerror)
	web.wget('http://google.fr', 'f3', onsuccess, onerror)
	print 'boom'
end

function onsuccess(filename)
	print(filename, 'wgetted!')
	print(io.open(filename):read('*all'))
end
function onerror(filename)
	print(filename, 'ko')
end

function drystal.update()
	drystal.stop()
end

