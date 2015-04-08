local drystal = require 'drystal'

local sys1 = drystal.new_system(200, 300, 5)
sys1:add_size(0, 6)
sys1:add_size(1, 6)
sys1:add_color(0, 0, 255, 0, 255, 0, 255)
sys1:add_color(1, 0, 255, 0, 255, 0, 255)
local running

local scrolling = false
local scrollx, scrolly = 0, 0

local systems = {
	drystal.new_system(400, 300),
}

function drystal.init()
	drystal.resize(600, 600)
	sys1:start()
	running = true
	for _, s in ipairs(systems) do
		s:start()
	end
end

function drystal.update(dt)
	if dt > .06 then
		dt = .06
	end
	sys1:update(dt)
	for _, s in ipairs(systems) do
		s:update(dt)
	end
end

function drystal.draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	sys1:draw(scrollx, scrolly)
	for _, s in ipairs(systems) do
		s:draw(scrollx, scrolly)
	end
end

function drystal.mouse_motion(x, y, dx, dy)
	sys1:set_position(x - scrollx, y - scrolly)
	if scrolling then
		scrollx = scrollx + dx
		scrolly = scrolly + dy
	end
end

function drystal.mouse_press(x, y, b)
	if b == drystal.buttons.left then
		local s = drystal.new_system(x - scrollx, y - scrolly)
		s:set_direction(0, math.pi/2)
		s:add_size(0, 6)
		s:add_size(1, 6)
		s:add_color(0, 0, 0, 0)
		s:add_color(1, 255, 255, 255)
		s:start()
		table.insert(systems, s)
		local s = s:clone()
		s:set_direction(math.pi, math.pi + math.pi/2)
		table.insert(systems, s)
	elseif b == drystal.buttons.right then
		scrolling = true
	elseif b == drystal.buttons.wheel_up then
		sys1:set_emission_rate(sys1:get_emission_rate() + 1)
	elseif b == drystal.buttons.wheel_down then
		sys1:set_emission_rate(sys1:get_emission_rate() - 1)
	end
end

function drystal.mouse_release(_, _, b)
	if b == drystal.buttons.right then
		scrolling = false
	end
end

function drystal.key_press(k)
	if k == 'space' then
		if running then
			sys1:stop()
		else
			sys1:start()
		end
		running = not running
	elseif k == 'c' then
		systems = {}
	elseif k == 'a' then
		drystal.stop()
	end
end

