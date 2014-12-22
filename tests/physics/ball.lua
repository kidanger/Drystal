local drystal = require 'drystal'

local ground, ground2, ball, ball2
local joint
local mouse_joint
local normal_sys
local boxes = {}

local R = 64 -- _ pixels = 1 meter

local Body = {
	num_collide=0,
}
Body.__index = Body
setmetatable(drystal.Body, Body)

function Body:begin_collide(other)
	self.num_collide = self.num_collide + 1
end
function Body:end_collide(other)
	self.num_collide = self.num_collide - 1
end

local function create_box(w, h, args, dynamic)
	local shape = drystal.new_shape("box", w, h)
	for k, v in pairs(args) do
		shape['set_' .. k](shape, v)
	end
	local box = drystal.new_body(dynamic, shape)
	box.width = w
	box.height = h
	function box:draw()
		local x, y = self:get_position()
		if self.num_collide > 0 then
			drystal.set_color(150, 0, 0)
		end
		drystal.draw_rect_rotated(x, y, w, h,
								self:get_angle(), 0, 0)
	end
	return box
end
local function create_circle(radius, args, dynamic)
	local shape = drystal.new_shape("circle", radius)
	for k, v in pairs(args) do
		shape['set_' .. k](shape, v)
	end
	local circle = drystal.new_body(dynamic, shape)
	circle.radius = radius
	function circle:draw()
		local angle = self:get_angle()
		x, y = self:get_position()
		if self.num_collide > 0 then
			drystal.set_color(150, 0, 0)
		end
		drystal.draw_circle(x, y, self.radius)
		drystal.set_color(150, 150, 150)
		drystal.draw_line(x, y, x + self.radius*math.cos(angle),
							y + ball.radius * math.sin(angle))
	end
	circle.p_system = drystal.new_system(0, 0)
	circle.p_system:set_min_initial_acceleration(-5)
	circle.p_system:set_max_initial_acceleration(-5)
	circle.p_system:set_min_lifetime(1)
	circle.p_system:set_max_lifetime(1.5)
	circle.p_system:start()
	return circle
end

function drystal.init()
	drystal.resize(600, 400)

	drystal.init_physics(0, 0.98, R)
	local gx, gy = drystal.get_gravity()
	assert(gx == 0)
	assert(gy == 0.98)
	drystal.set_gravity(128, 56)
	gx, gy = drystal.get_gravity()
	assert(gx == 128)
	assert(gy == 56)
	drystal.set_gravity(0, 0.98)

	-- create ground
	ground = create_box(6*R, .2*R, {friction=5}, false)
	ground:set_position(2*R, 4.5*R)

	ground2 = create_box(2*R, .1*R, {}, false)
	ground2:set_position(2*R, 2*R)
	ground2:set_angle(math.pi/12)

	-- create ball
	ball = create_circle(0.2*R, {restitution=0.4}, true)
	ball:set_position(3.5*R, 4*R)

	ball2 = create_circle(0.2*R, {restitution=0.4}, true)
	ball2:set_position(4*R, 0*R)
	ball2.immune = false
	function ball2:collide_with()
		return not self.immune
	end

	joint = drystal.new_joint('distance', ball2, ball)
	joint:set_length(100)
	joint:set_frequency(0.9)

	joint2 = drystal.new_joint('friction', ball, ground2, 0, 0, 0, 0)
	joint2:set_max_torque(2)
	joint2:set_max_force(0.2)
	assert(joint2:get_max_torque() == 2)
	assert(joint2:get_max_force() == 0.2)

	for i = 1, 600/30 do
		local b = create_box(30, 30, {}, true)
		local x = i * 32
		local y = 0
		b:set_position(x, y)
		table.insert(boxes, b)
		b.begin_collide = function()
			if math.random() < 0.05 then
				b:destroy()
				for i, v in ipairs(boxes) do
					if v == b then
						table.remove(boxes, i)
					end
				end
			end
		end
	end

	function presolve(b1, b2, x, y, dx, dy)
		normal_sys:set_position(x, y)
		normal_sys:set_direction(math.atan2(dy, dx))
		normal_sys:emit()
		local collide = true
		if b1.collide_with and not b1:collide_with(b2) then
			collide = false
		elseif b2.collide_with and not b2:collide_with(b1) then
			collide = false
		end
		return collide
	end

	drystal.on_collision(
		function (b1, b2)
			if b1.begin_collide then b1:begin_collide(b2) end
			if b2.begin_collide then b2:begin_collide(b1) end
		end,
		function (b1, b2)
			if b1.end_collide then b1:end_collide(b2) end
			if b2.end_collide then b2:end_collide(b1) end
		end,
		presolve
	)

	normal_sys = drystal.new_system(0, 0)
	normal_sys:set_lifetime(1)
	normal_sys:add_size(0, 2)
	normal_sys:add_size(1, 1)
	normal_sys:add_color(0, 0, 0, 0)
	normal_sys:add_color(1, 0, 0, 0)
end

local dir = ''
local time = 0
function drystal.update(dt)
	if dt > .6 then
		dt = .6
	end
	drystal.update_physics(dt)
	time = time + dt

	if dir == 'left' then
		ball:set_angular_velocity(-6)
	elseif dir == 'right' then
		ball:set_angular_velocity(6)
	else
		ball:set_angular_velocity(0)
	end

	local function update_system(ball)
		local x, y = ball:get_position()
		ball.p_system:set_position(x, y)
		local dx, dy = ball:get_linear_velocity()
		ball.p_system:set_min_direction(math.atan2(dy, dx) - math.pi/12)
		ball.p_system:set_max_direction(math.atan2(dy, dx) + math.pi/12)
		ball.p_system:set_min_initial_velocity((dx+dy))
		ball.p_system:set_max_initial_velocity((dx+dy))
		if ball.num_collide > 0 then
			ball.p_system:set_emission_rate(10)
		else
			ball.p_system:set_emission_rate(1)
		end
		ball.p_system:update(dt)
	end
	update_system(ball)
	update_system(ball2)
	normal_sys:update(dt)
end

function drystal.draw()
	drystal.set_color(120, 120, 120)
	drystal.draw_background()

	drystal.set_color(0, 0, 0)

	ground:draw()
	drystal.set_color((math.sin(time)*.5 + .5)*120, 120, 102)
	ground2:draw()
	drystal.set_color(0, 0, 0)

	ball:draw()
	ball2:draw()

	drystal.set_color(0, 0, 0)
	local x1, y1 = ball:get_position()
	local x2, y2 = ball2:get_position()
	drystal.draw_line(x1, y1, x2, y2)

	for _, b in ipairs(boxes) do
		b:draw()
	end

	do -- raycast
		local dx, dy = ball:get_linear_velocity()
		local angle = math.atan2(dy, dx)
		local xx = x1 + math.cos(angle) * 200
		local yy = y1 + math.sin(angle) * 200
		local _, points = drystal.raycast(x1, y1, xx, yy, 'all')
		for _, p in ipairs(points) do
			drystal.set_color(drystal.colors.darkgreen)
			local x, y = unpack(p)
			drystal.draw_point(x, y, 10)
		end
		local body, cx, cy = drystal.raycast(x1, y1, xx, yy, 'closest')
		if body then
			drystal.set_color(drystal.colors.orange)
			xx = cx
			yy = cy
		end
		drystal.draw_line(x1, y1, xx, yy)
	end

	ball.p_system:draw()
	ball2.p_system:draw()
	normal_sys:draw()
end

function drystal.key_press(key)
	if key == 'space' then
		ball:apply_linear_impulse(0, -0.3)
	elseif key == 'a' then
		ball2.immune = not ball2.immune
	elseif key == 'q' then
		ball:apply_angular_impulse(0.02)
	elseif key == 'left' then
		dir = key
	elseif key == 'right' then
		dir = key
	end
end
function drystal.key_release(key)
	if key == 'left' then
		dir = ''
	elseif key == 'right' then
		dir = ''
	end
end

function drystal.mouse_release(x, y, b)
	if b == 1 then
		mouse_joint:destroy()
		mouse_joint = nil
	end
end
function drystal.mouse_motion(x, y)
	if mouse_joint then
		mouse_joint:set_target(x, y)
	end
end
function drystal.mouse_press(x, y, b)
	if b == drystal.BUTTON_LEFT then
		if not mouse_joint then
			mouse_joint = drystal.new_joint('mouse', ground, ball, 50*ball:get_mass(), true)
		end
		mouse_joint:set_target(x, y)
	end
	if b == drystal.BUTTON_RIGHT then
		ball:set_position(x, y)
		ball:set_angular_velocity(0)
		ball:set_linear_velocity(0, 0)
	end
end

function drystal.atexit()
	joint:destroy()
end
