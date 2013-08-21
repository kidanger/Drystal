print = require 'colorprint'
require 'drystal'
local particle = require 'particle'

local sys1 = particle.new_system(200, 300)
local sys2 = particle.new_system(400, 300)

function init()
	resize(600, 600)
end

function update(dt)
	if dt > 100 then
		dt = 100
	end
	sys1:update(dt / 1000)
	sys2:update(dt / 1000)
end

function draw()
	set_color(255, 255, 255)
	draw_background()

	sys1:draw()
	sys2:draw()

	flip()
end

function mouse_motion(x, y)
	sys1:set_position(x, y)
end

function key_press(k)
	if k == 'a' then
		particle.free(sys1);
		particle.free(sys2);
		engine_stop()
	end
end
