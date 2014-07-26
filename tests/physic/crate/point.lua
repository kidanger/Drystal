module(..., package.seeall)

local methods = {}

local instance = {__index=methods}

function new(x,y)
	local tbl = {x=x, y=y}
	return setmetatable(tbl, instance)
end

north = new(0, -1) ; up = north
south = new(0, 1) ; down = south
west = new(-1, 0) ; left = west
east = new(1, 0) ; right = east

local mt = getmetatable(_M)
mt.__call=function(t,x,y)
	return t.new(x,y)
end

function methods:copy()
	return new(self.x, self.y)
end

function methods:ortho(pt2)
	return self.x == pt2.x or self.y == pt2.y
end

function methods:toward(pt2)
	if not self:ortho(pt2) then error(self .. ' not in a straight line with ' .. pt2)
	else
		local v = pt2 - self
		if v.x > 0 then v.x=1 end
		if v.x < 0 then v.x=-1 end
		if v.y > 0 then v.y=1 end
		if v.y < 0 then v.y=-1 end
		return v
	end
end

function methods:adjacent(pt2)
	local d = pt2-self
	return (d.x == 0 or d.y == 0) and (math.abs(d.x+d.y) == 1)
end

function instance.__add(pt1, pt2)
	assert(pt1 and pt2)
	return new(pt1.x+pt2.x, pt1.y+pt2.y)
end

function instance.__sub(pt1, pt2)
	assert(pt1 and pt2)
	return new(pt1.x-pt2.x, pt1.y-pt2.y)
end

function instance.__mul(pt1, pt2)
	assert(pt1 and pt2)
	if type(pt2) == 'number' then
		return new(pt1.x * pt2, pt1.y * pt2)
	else
		return new(pt1.x*pt2.x, pt1.y*pt2.y)
	end
end

methods.translate = instance.__add

function instance.__tostring(pt)
	return string.format('(%d, %d)', pt.x, pt.y)
end

function instance.__call(pt)
	return pt.x, pt.y
end

function instance.__eq(pt1, pt2)
	return pt1.x == pt2.x and pt1.y == pt2.y
end

-- A point is "less than" a point if each
-- coord is less than the corresponding one
function instance.__lt(pt1, pt2)
	return pt1.x < pt2.x and pt1.y < pt2.y
end

function instance.__le(pt1, pt2)
	return pt1.x <= pt2.x and pt1.y <= pt2.y
end

function test()
	local p = point(2,3)
	assert(p.x == 2 and p.y == 3)
	assert(tostring(p) == "(2, 3)")
	p = p + point(1,1)
	assert(tostring(p) == "(3, 4)")
	local p2 = p:copy()
	p2.y = p2.y-1
	assert(tostring(p) == "(3, 4)")
	assert(tostring(p2) == "(3, 3)")
	assert(p2 + point(1, 1) == point(4, 4))

	local o1, o2 = point(3, 3), point(3, 5)
	assert(o1:ortho(o2))
	assert(o2-o1 == point(0, 2))
	assert(o1:toward(o2) == point(0, 1))

	local a1, a2, a3 = point(2, 2), point(1, 2), point(3, 3)
	assert(a1:adjacent(a2))
	assert(a2:adjacent(a1))
	assert(not a2:adjacent(a3))
	assert(not a1:adjacent(a3))
	assert(not a1:adjacent(a1))

	assert(a2 <= a1)
	assert(a1 < a3)
	assert(a3 > a1)
	assert(not(a2 < a1))
end

test()
