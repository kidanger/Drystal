package.path = 'data/?.lua;' .. package.path
package.cpath = 'data/?.so;' .. package.cpath

require 'drystal'
require 'web'

function init()
	print(is_web())
	wget('/src/log.hpp', 'lol', onsuccess, onerror)
	wget('src/log.hpp', 'kk', onsuccess, onerror)
	wget('http://google.fr', 'k2', onsuccess, onerror)
	print 'boom'
end

function onsuccess(filename)
	print(filename, 'wgetted!')
	print(io.open(filename):read('*all'))
end
function onerror(filename)
	print(filename, 'ko')
end

function update()
	engine_stop()
end
