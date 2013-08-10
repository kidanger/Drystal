require 'drystal'
local physic = require 'physic'

local ground, ground2, ball

local R = 64 -- _ pixels = 1 meter

local function create_box(w, h, dynamic)
	local shape = physic.new_shape("box", w, h)
	local box = physic.new_body(shape, dynamic)
	box.width = w
	box.height = h
	function box:draw()
		local x, y = self:get_position()
		x = x * R
		y = y * R
		draw_rect_rotated(x - (w / 2) * R,
						y - (h / 2) * R,
						w * R, h * R,
						self:get_angle())
	end
	return box
end
local function create_circle(radius, args, dynamic)
	local shape = physic.new_shape("circle", radius)
	for k, v in pairs(args) do
		shape['set_' .. k](shape, v)
	end
	local circle = physic.new_body(shape, dynamic)
	circle.radius = radius
	return circle
end

function init()
	resize(600, 400)

	physic.create_world(0, 0.98)

	-- create ground
	ground = create_box(5, .2, false)
	ground:set_position(5, 4.5)

	ground2 = create_box(2, .1, false)
	ground2:set_position(3.5, 2.5)
	ground2:set_angle(math.pi/12)

	-- create ball
	ball = create_circle(0.2, {restitution=.8}, true)
	ball:set_position(3.5, 0)
end

local time = 0
function update(dt)
	delta = dt / 1000
	physic.update(delta * 2)
	time = time + delta
end

function draw()
	set_color(120, 120, 120)
	draw_background()

	set_color(0, 0, 0)

	ground:draw()
	set_color(math.sin(time)*120, 120, 102)
	ground2:draw()
	set_color(0, 0, 0)

	x, y = ball:get_position()
	draw_circle(x * R, y * R, ball.radius * R)

	flip()
end

