print = require 'colorprint'
local drystal = require 'drystal'
local particle = require 'particle'

local sys1 = particle.new_system(100, 550)

sys1:add_size(0, 30)
sys1:add_size(.2, 20, 25)
sys1:add_size(1, 5)

sys1:add_color(0, 255, 255, 255)
sys1:add_color(0.1, 255, 0, 0)
sys1:add_color(0.4, 255, 100, 75)
sys1:add_color(1, 0, 0, 0)

sys1:set_lifetime(4)

sys1:set_direction(- math.pi / 2 - math.pi/12, -math.pi/2 + math.pi/12)
sys1:set_initial_velocity(100)
sys1:set_initial_acceleration(0)

function init()
	drystal.resize(600, 600)
	sys1:start()
end

function update(dt)
	dt = dt / 1000
	if dt > .06 then
		dt = .06
	end
	sys1:update(dt)
end

function draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	sys1:draw()

	drystal.flip()
end

function key_press(k)
	if k == 'a' then
		drystal.engine_stop()
	end
end

