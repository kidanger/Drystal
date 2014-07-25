local drystal = require 'drystal'

if drystal.is_web then
	drystal.raw_wget = drystal.wget
	local wget_requests = {}

	function drystal.on_wget_success(file)
		for _, f in ipairs(wget_requests[file].onload) do
			f(file)
		end
		wget_requests[file] = nil
	end
	function drystal.on_wget_error(file)
		for _, f in ipairs(wget_requests[file].onerror) do
			f(file)
		end
		wget_requests[file] = nil
	end

	function drystal.wget(url, file, onload, onerror)
		assert(onload)
		assert(onerror)

		if not drystal.is_web then
			return
		end

		if wget_requests[file] then
			-- already requested
			table.insert(wget_requests[file].onload, onload)
			table.insert(wget_requests[file].onerror, onerror)
			return
		end

		drystal.raw_wget(url, file)
		wget_requests[file] ={
			onload={onload},
			onerror={onerror},
		}
	end
end

