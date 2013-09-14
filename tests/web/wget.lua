local drystal = require 'drystal'
local web = require 'web'

function drystal.init()
	print(web.is_web())
	web.wget('/src/log.hpp', 'lol', onsuccess, onerror)
	web.wget('src/log.hpp', 'kk', onsuccess, onerror)
	web.wget('http://google.fr', 'k2', onsuccess, onerror)
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
	drystal.engine_stop()
end
