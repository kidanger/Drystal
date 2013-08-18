require 'drystal'
local physic = require 'physic'

require 'point'

function init()
	math.randomseed(os.time())
	resize(600, 400)
	physic.create_world(0, 0)
	-- physic.set_ratio(32)

	crates = { makeCrate(5, 5),
	makeCrate(5, 6) }
	player = makePlayer()
end

function makeCrate(x, y)
	local s = physic.new_shape('box', 32, 32)
	local b = physic.new_body(s, 'dynamic')
	b:set_position(x*32 + 16, y*32 + 16)
	b:set_fixed_rotation(true)
	b:set_linear_damping(4)
	return {body=b, shape=s}
end

function makePlayer()
	local s = physic.new_shape('circle', 16)
	local b = physic.new_body(s, 'dynamic')
	b:set_position(48, 48)
	return {body=b, shape=s}
end

function draw()
	set_color(64, 120, 64)
	draw_background()

	-- Draw player
	set_color(160, 64, 64)
	local x, y = player.body:get_position()
	draw_circle(x, y, 16)

	-- Draw crates
	set_color(180, 120, 90)
	for _, c in ipairs(crates) do
		x, y = c.body:get_position()
		draw_rect(x-16, y-16, 32, 32)
	end

	flip()
end

--------------------------------------------------

local keys = {}
function key_press(k)
	keys[k] = true
end
function key_release(k)
	keys[k] = nil
end
local function key_is_down(k)
	return keys[k] ~= nil
end
function update(dt)
	local k = key_is_down
	local kd = 0
	local dir = point(0, 0)

	if k('up') then dir = dir + point.up ; kd = kd + 1 end
	if k('down') then dir = dir + point.down ; kd = kd + 1 end
	if k('left') then dir = dir + point.left ; kd = kd + 1 end
	if k('right') then dir = dir + point.right ; kd = kd + 1 end

	if kd > 0 then
		player.body:set_linear_damping(0)
	else
		player.body:set_linear_damping(8)
	end

	max_speed(player.body, 320)

	local f = dir * 320
	player.body:apply_force(f.x, f.y)

	if kd == 1 then
		dampenSidewaysVelocity(player.body, dir, dt)
	end

	for _, c in ipairs(crates) do
		local x, y = c.body:get_position()
		local sq = point(math.floor(x / 32), math.floor(y / 32))
		nudgeToSquare(c.body, sq, 20)
	end

	physic.update(dt)
end

function max_speed(body, spd)
	local x, y = body:get_linear_velocity()
	if x*x + y*y > spd*spd then
		local a = math.atan2(y,x)
		body:set_linear_velocity(spd * math.cos(a), spd * math.sin(a))
	end
end

function dampenSidewaysVelocity(body, dir, dt)
	local a = 1 - 4 * dt
	if a > 1.0 then a = 1.0 elseif a < 0 then a = 0 end
	local v = point(body:get_linear_velocity())

	if dir.y == 0 then v.y = v.y * a end
	if dir.x == 0 then v.x = v.x * a end

	body:set_linear_velocity(v())
end

function nudgeToSquare(body, sq, acc)
	local x, y = body:get_position()
	x = x - 16
	y = y - 16
	local ty = sq.y * 32
	local f = acc * (ty - y)
	body:apply_force(0, f)

	local tx = sq.x * 32
	local f = acc * (tx - x)
	body:apply_force(f, 0)
end

function mouse_press(x, y)
	table.insert(crates, makeCrate(x/32, y/32))
end
