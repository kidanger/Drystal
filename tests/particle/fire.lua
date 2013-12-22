local drystal = require 'drystal'
local particle = require 'particle'

if not sys1 then -- for livecoding
sys1 = particle.new_system(100, 550)

sys1:add_size(0, 15)
sys1:add_size(.2, 7, 10)
sys1:add_size(1, 5)

sys1:add_color(0, 255, 0, 0)
sys1:add_color(0.4, 255, 100, 75)
sys1:add_color(1, 0, 0, 0)
end

sys1:set_lifetime(3)

sys1:set_direction(- math.pi / 2 - math.pi/12, -math.pi/2 + math.pi/12)
sys1:set_initial_velocity(100)
sys1:set_initial_acceleration(0)
sys1:set_emission_rate(100)

function drystal.init()
	drystal.resize(600, 600)
	sys1:start()
end

function drystal.update(dt)
	dt = dt / 1000
	if dt > .06 then
		dt = .06
	end
	sys1:update(dt)
end

function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	sys1:draw()

	drystal.flip()
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

