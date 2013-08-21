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
	particle.update(sys1, dt/1000)
	particle.update(sys2, dt/1000)
end

function draw()
	set_color(255, 255, 255)
	draw_background()

	particle.draw(sys1)
	particle.draw(sys2)

	flip()
end

function key_press(k)
	if k == 'a' then
		particle.free(sys1);
		particle.free(sys2);
		engine_stop()
	end
end
