local drystal = require 'drystal'

drystal.raw_raycast = drystal.raycast

local function any(body, fraction)
	return 0, true
end

local function closest(body, fraction)
	return fraction, true
end

local function farthest(destx, desty)
	local distance

	return function(body, fraction, x, y)
		local d = math.distance(destx, desty, x, y)
		if not distance or d < distance then
			distance = d
			return 1, true
		end
		return 1, false
	end
end

local function all(bodies, points)
	return function(body, fraction, x, y)
		table.insert(bodies, body)
		table.insert(points, {x, y})
		return 1, fraction
	end
end

function drystal.raycast(x1, y1, x2, y2, method)
	if method == 'any' then
		method = any
	elseif method == 'closest' then
		method = closest
	elseif method == 'farthest' or method == 'furthest' then
		method = farthest(x2, y2)
	elseif method == 'all' then
		local bodies = {}
		local points = {}
		drystal.raw_raycast(x1, y1, x2, y2, all(bodies, points))
		return bodies, points
	end
	return drystal.raw_raycast(x1, y1, x2, y2, method)
end

