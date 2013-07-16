require "data/drystal"

UP = -1
DOWN = 1
R = 4

TICK_SPEED = 1000 / 60

local debug = false
local right, left, up, down, space = false, false, false, false, false
local pause = false

local types = {'explosive', 'deadround', }

Ship = Ship or {}
Ship.__index = Ship

function mrand(min, max)
	local s = math.random()
	return min + s * (max - min)
end

function math.sign(t)
	if t > 0 then return 1 end
	if t < 0 then return -1 end
	return 0
end
function normalize(x, y)
	local d = math.sqrt(x*x + y*y)
	return x / d, y / d
end
function copy(orig)
    local orig_type = type(orig)
    local _copy
    if orig_type == 'table' then
        _copy = {}
        for orig_key, orig_value in next, orig, nil do
            _copy[copy(orig_key)] = copy(orig_value)
        end
        setmetatable(_copy, copy(getmetatable(orig)))
    else -- number, string, boolean, etc
        _copy = orig
    end
    return _copy
end

BulletSpeedLinear = {
	speed_min = .4,
	speed_max = .4,
	accel_min = 1.0,
	accel_max = 1.0,
}
function BulletSpeedLinear:apply(owner, b)
	local speed = mrand(self.speed_min, self.speed_max)
	local dx, dy = normalize(b.dx, b.dy)
	dx = dx * speed
	dy = dy * speed
	b.dx, b.dy = dx, dy

	local accel = mrand(self.accel_min, self.accel_max)
	b.ax = accel
	b.ay = accel
end
function BulletSpeedLinear:update(owner, b, dt)
	b.x = b.x + b.dx * dt
	b.dx = b.dx * b.ax

	b.y = b.y + b.dy * dt
	b.dy = b.dy * b.ay
end

BulletFireLine = {}
BulletFireCircle = {
	bullet_per_rev_min = 10,
	bullet_per_rev_max = 50,
}
function BulletFireCircle:fire(owner)
	local step = 360/mrand(self.bullet_per_rev_min, self.bullet_per_rev_max)
	local begin = mrand(0, 360)
	for a = begin, begin+360, step do
		local dx = math.cos(math.rad(a))
		local dy = math.sin(math.rad(a))
		local b = {
			dx = dx,
			dy = dy,
		}
		owner:add_bullet(b)
	end
end
function BulletFireLine:fire(owner)
	local b = {
		dx = 0,
		dy = owner.dir,
	}
	owner:add_bullet(b)
end

BulletShapeCircle = {
	radius = 3
}
BulletShapeRect = {
	w = 2,
	h = 6
}
function BulletShapeCircle:draw(owner, b)
	draw_circle(b.x, b.y, self.radius)
end
function BulletShapeRect:draw(owner, b)
	draw_rect(b.x - self.w/2, b.y - self.h/2,
				self.w, self.h)
end

function Ship.init()
	local s = 32
	Ship.surface = new_surface(s, s)
	draw_on(Ship.surface)
	set_color(255, 255, 255)
	set_alpha(255)
	draw_rect(s*.1, s*.2, s*.8, s*.6)
	draw_circle(s*.5, s*.6, s*.3)
	set_alpha(200)
	draw_rect(s*.2, 0, s*.2, s*.4)
	draw_rect(s*.8-s*.2, 0, s*.2, s*.4)
	set_alpha(255)
end

function Ship.new()
	local s = setmetatable({}, Ship)
	s.x, s.y = 0, 0
	s.dx, s.dy = 0, 0
	s.fire = false
	s.health = 255
	s.bullets = {}
	s.dir = DOWN
	s.next_fire = 0
	s.tick = 0
	s.heat = 0
	s.size = 32
	s.player = false
	s.rlfire = 0
	s.firetype = BulletFireCircle
	s.bshapetype = BulletShapeCircle
	s.bspeedtype = BulletSpeedLinear
	return s
end

function Ship:draw()
	local a = math.atan2(self.dx, -self.dy)
	if self.dx == 0 and self.dy == 0 then
		a = math.atan2(0, -self.dir)
	end
	rotate_surface(Ship.surface, math.deg(a))
	resize_surface(Ship.surface, self.size, self.size)
	set_color(255 - self.heat*.2, 255 - self.heat*.8, 255 - self.heat*.8)
	draw_surface(Ship.surface, self.x-32/2, self.y-32/2)
	if debug then
		set_fill(false)
		draw_rect(self.x-self.size/2, self.y-self.size/2, self.size, self.size)
		set_fill(true)
	end
	if self.player then
		set_color(100, 230, 120)
		draw_circle(self.x, self.y, R)
	end
end

function Ship:draw_bullets()
	for _, b in ipairs(self.bullets) do
		self.bshapetype:draw(self, b)
	end
end

function Ship:update(dt)
	self.tick = self.tick + 1
	self.heat = math.max(0, self.heat - 5)
	local destx = self.x + self.dx * dt * 0.3
	if self.x ~= destx then
		self.x = math.min(width, math.max(0, destx))
	end
	local desty = self.y + self.dy * dt * 0.3
	if self.y ~= desty then
		self.y = math.min(current_y+height, math.max(current_y, desty))
	end
	if self.fire and self.tick >= self.next_fire then
		self:shot()
		self.next_fire = self.tick + self.rlfire
	end
	self:update_bullets(dt)
end

function Ship:update_bullets(dt)
	for i = #self.bullets, 1, -1 do
		local b = self.bullets[i]
		if b.y < current_y or b.y > current_y+height
			or b.x < 0 or b.x > width then
			table.remove(self.bullets, i)
		end
	end
	for _, b in ipairs(self.bullets) do
		self.bspeedtype:update(self, b, dt)
	end
end

function Ship:shot()
	self.firetype:fire(self)
end

function Ship:add_bullet(b)
	b.x = self.x
	b.y = self.y
	if b.dx == 0 and b.dy == 0 then
		b.dy = self.dir
	end
	self.bspeedtype:apply(self, b)
	self.bullets[#self.bullets+1] = b
end

function Ship:hit(other, bullet_idx)
	local b = self.bullets[bullet_idx]
	table.remove(self.bullets, bullet_idx)
	other.health = math.max(0, other.health - 60)
	other.heat = math.min(255, other.heat + 150)
end

function create_ship(type)
	local s = Ship:new()
	s.rlfire = 1.5 * TICK_SPEED
	s.firetype = copy(BulletFireCircle)
	s.bspeedtype = copy(BulletSpeedLinear)
	s.bshapetype = BulletShapeRect
	if type == 'explosive' then
		s.bspeedtype.speed_min = 0.1
		s.bspeedtype.speed_max = 0.15
		s.bspeedtype.accel_min = 1.01
	elseif type == 'deadround' then
		s.bspeedtype.speed_min = 0.1
		s.bspeedtype.speed_max = 0.1
		s.firetype.bullet_per_rev_min = 20
		s.firetype.bullet_per_rev_max = 40
	else
		print ('unknown type', type)
	end
	return s
end

function init()
	collectgarbage ('step', 10)
	set_fill(true)
	set_alpha(255)
	width, height = surface_size(screen)

	Ship.init()

	current_y = 0

	ship = Ship.new()
	ship.y = current_y + height - 20
	ship.x = width / 2
	ship.dir = UP
	ship.player = true
	ship.rlfire = 0.15 * TICK_SPEED
	ship.firetype = BulletFireLine

	enemies = {}
	draw_on(screen)
	display_logo({40, 40, 40})
end

local tick = 0
old = 0
function update(dt)
	tick = tick + 1

	if pause then
		return
	end

	ship:update(dt)

	if left then ship.dx = -1
	elseif right then ship.dx = 1
	else ship.dx = 0 end
	if up then ship.dy = -1
	elseif down then ship.dy = 1
	else ship.dy = 0 end
	ship.fire = space

	local dy = math.ceil(dt / 10)
	current_y = current_y - dy
	ship.y = ship.y - dy

	for i = #enemies, 1, -1 do
		local e = enemies[i]
		if e.health == 0
			or e.x < 0 or e.x > width
			or e.y > current_y+height then
			table.remove(enemies, i)
		end
	end
	for _, s in ipairs(enemies) do
		update_ai(s)
		s:update(dt)
	end

	if math.floor(tick) % 60 == 0 then
		local s = create_ship(types[math.random(1, #types)])
		s.x = math.random(width)
		s.y = current_y + math.random(0, 20)
		s.size = math.random(32, 64)
		enemies[#enemies + 1] = s
	end

	local abs = math.abs
	for bi, b in ipairs(ship.bullets) do
		for _, e in ipairs(enemies) do
			if abs(b.x - e.x) < e.size/2 and abs(b.y - e.y) < e.size/2 then
				ship:hit(e, bi)
				break
			end
		end
	end
	for _, e in ipairs(enemies) do
		for bi, b in ipairs(e.bullets) do
			if abs(b.x - ship.x) < R and abs(b.y - ship.y) < R then
				e:hit(ship, bi)
			end
		end
	end
	if tick % 60 == 0 then
		new = collectgarbage('count')
		print('mem', new, 'diff', new - old)
		old = new
	end
end

function update_ai(s)
	if math.random(0, 20) == 0 then
		s.fire = true
	elseif math.random(0, 5) == 0 then
		s.fire = false
	end
end

function draw()
	set_color(40, 40, 40)
	draw_background()
	push_offset(0, -current_y)

	ship:draw()

	for _, s in ipairs(enemies) do
		s:draw()
	end

	set_color(255, 255, 255)
	ship:draw_bullets()
	for _, s in ipairs(enemies) do
		s:draw_bullets()
	end

	pop_offset()
	flip()
end


function key_release(key)
	if key == 'q' then
		left = false
	elseif key == 'd' then
		right = false
	elseif key == 'z' then
		up = false
	elseif key == 's' then
		down = false
	elseif key == 'space' then
		space = false
	end
end
function key_press(key)
	if key == 'q' then
		left = true
	elseif key == 'd' then
		right = true
	elseif key == 'z' then
		up = true
	elseif key == 's' then
		down = true
	elseif key == 'space' then
		space = true
	end
	if key == 'g' then
		debug = not debug
	end
	if key == 'a' then
		engine_stop()
	end
	if key == 'p' then
		pause = not pause
	end
end

function mouse_motion(x, y)
	if debug then
		ship.x = x
		ship.y = current_y + y
	end
end
