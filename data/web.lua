-- this file acts as a proxy
-- without it, 'web' extension is still usable
-- but it offers facilities to manipulate wget
-- old wget is still accessible via 'web.raw_wget'

-- remove Lua loading to make sure shared lib is loaded
local _path = package.path
package.path = ''

local web = require 'web'
package.path = _path

web.raw_wget = web.wget
local wget_requests = {}

function on_wget_success(file)
	for _, f in ipairs(wget_requests[file].onload) do
		f(file)
	end
	wget_requests.file = nil
end
function on_wget_error(file)
	for _, f in ipairs(wget_requests[file].onerror) do
		f(file)
	end
	wget_requests.file = nil
end

function web.wget(url, file, onload, onerror)
	if not onload or not onerror then
		print('invalid argument', onload, onerror)
	end
	if wget_requests[file] then
		-- already requested
		table.insert(wget_requests[file].onload, onload)
		table.insert(wget_requests[file].onerror, onerror)
		return
	end
	if not web.raw_wget(url, file) then
		return
	end
	wget_requests[file] ={
		onload={onload},
		onerror={onerror},
	}
end

return web
