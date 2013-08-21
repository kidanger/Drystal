print = require 'colorprint'
require 'drystal'
local particle = require 'particle'

local sys1 = particle.new_system(200, 300)

local systems = {
	particle.new_system(400, 300),
}

function init()
	resize(600, 600)
	sys1:start()
	for _, s in ipairs(systems) do
		s:start()
	end
end

function update(dt)
	dt = dt / 1000
	if dt > .06 then
		dt = .06
	end
	sys1:update(dt)
	for _, s in ipairs(systems) do
		s:update(dt)
	end
end

function draw()
	set_color(0, 0, 0)
	draw_background()

	sys1:draw()
	for _, s in ipairs(systems) do
		s:draw()
	end

	flip()
end

function mouse_motion(x, y)
	sys1:set_position(x, y)
end

function mouse_press(x, y, b)
	if b == 1 then
		s = particle.new_system(x, y)
		s:set_min_direction(0)
		s:set_max_direction(math.pi*2)
		s:start()
		table.insert(systems, s)
	elseif b == 4 then
		sys1:set_emission_rate(sys1:get_emission_rate() + 1)
	elseif b == 5 then
		sys1:set_emission_rate(sys1:get_emission_rate() - 1)
	end
end

function key_press(k)
	if k == 'space' then
		sys1:set_running(not sys1:is_running())
	end
	if k == 'a' then
		particle.free(sys1);
	for _, s in ipairs(systems) do
		s:free()
	end
		engine_stop()
	end
end
