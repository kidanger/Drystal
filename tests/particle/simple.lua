print = require 'colorprint'
require 'drystal'
local particle = require 'particle'

local sys1 = particle.new_system(200, 300)
sys1:add_size(0, 6)
sys1:add_size(1, 6)
sys1:add_color(0, 0, 255, 0, 255, 0, 255)
sys1:add_color(1, 0, 255, 0, 255, 0, 255)

local scrolling = false
local scrollx, scrolly = 0, 0

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

	sys1:draw(scrollx, scrolly)
	for _, s in ipairs(systems) do
		s:draw(scrollx, scrolly)
	end

	flip()
end

function mouse_motion(x, y, dx, dy)
	sys1:set_position(x - scrollx, y - scrolly)
	if scrolling then
		scrollx = scrollx + dx
		scrolly = scrolly + dy
	end
end

function mouse_press(x, y, b)
	if b == 1 then
		s = particle.new_system(x - scrollx, y - scrolly)
		s:set_direction(0, math.pi*2)
		s:add_size(0, 6)
		s:add_size(1, 6)
		s:add_color(0, 0, 0, 0)
		s:add_color(1, 255, 255, 255)
		s:start()
		table.insert(systems, s)
	elseif b == 3 then
		scrolling = true
	elseif b == 4 then
		sys1:set_emission_rate(sys1:get_emission_rate() + 1)
	elseif b == 5 then
		sys1:set_emission_rate(sys1:get_emission_rate() - 1)
	end
end

function mouse_release(_, _, b)
	if b == 3 then
		scrolling = false
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
