print = require 'colorprint'
local drystal = require 'drystal'
local physic = require 'physic'
local particle = require 'particle'

local ground, ground2, ball, ball2
local joint
local mouse_joint

local R = 64 -- _ pixels = 1 meter

local Body = setmetatable({
	num_collide=0,
}, physic.__body_class)
Body.__index = Body

function Body:begin_collide(other)
	self.num_collide = self.num_collide + 1
end
function Body:end_collide(other)
	self.num_collide = self.num_collide - 1
end

local function create_box(w, h, args, dynamic)
	local shape = physic.new_shape("box", w, h)
	for k, v in pairs(args) do
		shape['set_' .. k](shape, v)
	end
	local box = physic.new_body(shape, dynamic)
	box.width = w
	box.height = h
	function box:draw()
		local x, y = self:get_position()
		x = x * R
		y = y * R
		if self.num_collide > 0 then
			drystal.set_color(150, 0, 0)
		end
		drystal.draw_rect_rotated(x - (w / 2) * R,
								y - (h / 2) * R,
								w * R, h * R,
								self:get_angle())
	end
	return setmetatable(box, Body)
end
local function create_circle(radius, args, dynamic)
	local shape = physic.new_shape("circle", radius)
	for k, v in pairs(args) do
		shape['set_' .. k](shape, v)
	end
	local circle = physic.new_body(shape, dynamic)
	circle.radius = radius
	function circle:draw()
		local angle = self:get_angle()
		x, y = self:get_position()
		if self.num_collide > 0 then
			drystal.set_color(150, 0, 0)
		end
		drystal.draw_circle(x * R, y * R, self.radius * R)
		drystal.set_color(150, 150, 150)
		drystal.draw_line(x*R, y*R, x*R + self.radius*math.cos(angle)*R,
							y*R + ball.radius * math.sin(angle)*R)
	end
	circle.p_system = particle.new_system(0, 0)
	circle.p_system:set_min_initial_acceleration(-5)
	circle.p_system:set_max_initial_acceleration(-5)
	circle.p_system:set_min_lifetime(1)
	circle.p_system:set_max_lifetime(1.5)
	circle.p_system:start()
	return setmetatable(circle, Body)
end

function init()
	drystal.resize(600, 400)

	physic.create_world(0, 0.98)

	-- create ground
	ground = create_box(5, .2, {friction=5}, false)
	ground:set_position(5, 4.5)

	ground2 = create_box(2, .1, {}, false)
	ground2:set_position(3.5, 2.5)
	ground2:set_angle(math.pi/12)

	-- create ball
	ball = create_circle(0.2, {restitution=0.4}, true)
	ball:set_position(3.5, 0)

	ball2 = create_circle(0.2, {restitution=0.4}, true)
	ball2:set_position(4, 0)
	ball2.immune = false
	function ball2:collide_with()
		return not self.immune
	end

	joint = physic.new_joint('distance', ball2, ball)
	joint:set_length(100/R)
	joint:set_frequency(0.9)

--	joint = physic.new_joint('rope', ball2, ball, true)
--	joint:set_max_length(100/R)

	function presolve(b1, b2)
		local collide = true
		if b1.collide_with and not b1:collide_with(b2) then
			collide = false
		elseif b2.collide_with and not b2:collide_with(b1) then
			collide = false
		end
		return collide
	end

	physic.on_collision(
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
	end

local dir = ''
local time = 0
function update(dt)
	delta = dt / 1000
	if delta > .6 then
		delta = .6
	end
	physic.update(delta * 2)
	time = time + delta

	if dir == 'left' then
		ball:set_angular_velocity(-6)
	elseif dir == 'right' then
		ball:set_angular_velocity(6)
	else
		ball:set_angular_velocity(0)
	end

	local function update_system(ball)
		local x, y = ball:get_position()
		ball.p_system:set_position(x*R, y*R)
		local dx, dy = ball:get_linear_velocity()
		ball.p_system:set_min_direction(math.atan2(dy, dx) - math.pi/12)
		ball.p_system:set_max_direction(math.atan2(dy, dx) + math.pi/12)
		ball.p_system:set_min_initial_velocity((dx+dy)*R)
		ball.p_system:set_max_initial_velocity((dx+dy)*R)
		if ball.num_collide > 0 then
			ball.p_system:set_emission_rate(100)
		else
			ball.p_system:set_emission_rate(10)
		end
		ball.p_system:update(delta)
	end
	update_system(ball)
	update_system(ball2)
end

function draw()
	drystal.set_color(120, 120, 120)
	drystal.draw_background()

	drystal.set_color(0, 0, 0)

	ground:draw()
	drystal.set_color(math.sin(time)*120, 120, 102)
	ground2:draw()
	drystal.set_color(0, 0, 0)

	ball:draw()
	ball2:draw()

	drystal.set_color(0, 0, 0)
	local x1, y1 = ball:get_position()
	local x2, y2 = ball2:get_position()
	drystal.draw_line(x1*R, y1*R, x2*R, y2*R)

	ball.p_system:draw()
	ball2.p_system:draw()

	drystal.flip()
end

function key_press(key)
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
function key_release(key)
	if key == 'left' then
		dir = ''
	elseif key == 'right' then
		dir = ''
	end
end

function mouse_release(x, y, b)
	if b == 1 then
		mouse_joint:destroy()
		mouse_joint = nil
	end
end
function mouse_motion(x, y)
	if mouse_joint then
		mouse_joint:set_target(x/R, y/R)
	end
end
function mouse_press(x, y, b)
	if b == 1 then
		if not mouse_joint then
			mouse_joint = physic.new_joint('mouse', ground, ball, 7*ball:get_mass(), true)
		end
		mouse_joint:set_target(x/R, y/R)
	end
	if b == 3 then
		physic.on_collision()
		ball:set_position(x/R, y/R)
		ball:set_angular_velocity(0)
		ball:set_linear_velocity(0, 0)
	end
end
