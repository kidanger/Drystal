local drystal = require 'drystal'
local tt = require 'truetype'

local W, H = 600, 400
local target = 1 / 60
local number = 300

local draw_triangle = {name='draw_triangle'}
local draw_sprite_simple = {name='draw_sprite_simple'}
local draw_sprite_rotated = {name='draw_sprite_rotated'}
local draw_sprite_resized = {name='draw_sprite_resized'}
local draw_font_nocolor = {name='draw_font_nocolor'}
local draw_font_color = {name='draw_font_color'}
local state = {}
local states = { draw_triangle, draw_sprite_simple, draw_sprite_rotated, draw_sprite_resized, draw_font_nocolor, draw_font_color }
local states = { draw_font_nocolor, draw_font_color }
local current_state = 1
local number = 0
local tick = 0
local random = math.random
local sprite = {x=0, y=0, w=22, h=33}
local spritesheet = nil
local font = nil
local font_big = nil

function set_state(s)
	state = s
	s.max = 0
	if state.init then
		state:init()
	end
end

function draw_triangle:draw()
	drystal.set_color(0, 0, 0)
	drystal.draw_background()

	drystal.set_alpha(255)
	drystal.set_color(255, 0, 0)
	for i = 1, number do
		local x = random(W)
		local y = random(H)
		local w = random(20)
		local h = random(20)
		local w2 = random(20)
		local h2 = random(20)
		drystal.draw_triangle(x, y, x+w, y+h, x+w2, y+h2)
	end
end

function draw_sprite_simple:draw()
	drystal.set_color(255, 255, 255)
	drystal.set_alpha(255)
	drystal.draw_background()

	local random = math.random
	for i = 1, number do
		local x = random(W)
		local y = random(H)
		drystal.draw_sprite(sprite, x, y)
	end
end

function draw_sprite_rotated:draw()
	drystal.set_color(255, 255, 255)
	drystal.set_alpha(255)
	drystal.draw_background()

	local random = math.random
	for i = 1, number do
		local x = random(W)
		local y = random(H)
		local r = random(math.pi)
		drystal.draw_sprite_rotated(sprite, x, y, r)
	end
end

function draw_sprite_resized:draw()
	drystal.set_color(255, 255, 255)
	drystal.set_alpha(255)
	drystal.draw_background()

	local random = math.random
	for i = 1, number do
		local x = random(W)
		local y = random(H)
		local w = sprite.w*(random()+random(0,1))
		local h = sprite.h*(random()+random(0,1))
		drystal.draw_sprite_resized(sprite, x, y, w, h)
	end
end

function highlight(text, pos)
	return text:sub(0, pos-1) .. '{big|'.. text:sub(pos, pos) .. '}' .. text:sub(pos+1, #text)
end

function draw_font_nocolor:init()
	tt.use(font)
	tt.use_color(false)
end

function draw_font_nocolor:draw()
	drystal.set_alpha(255)
	drystal.set_color(200, 200, 200)
	drystal.draw_background()

	drystal.set_color(255, 0, 0)
	local text = 'abd bla test defghi'
	for i = 1, number do
		local x = random(W)
		local y = random(H)
		tt.draw_align(text, x, y, 'center')
	end
end

function draw_font_color:init()
	tt.use(font)
	tt.use_color(true)
end

function draw_font_color:draw()
	drystal.set_alpha(255)
	drystal.set_color(200, 200, 200)
	drystal.draw_background()

	drystal.set_color(255, 0, 0)
	local text = 'abd {r:0|outline|big|b:150|bla} {small|test} {big|%:50|defghi}'
	for i = 1, number do
		local x = random(W)
		local y = random(H)
		tt.draw_align(text, x, y, 'center')
	end
end

function drystal.init()
	drystal.resize(W, H)
	font = assert(tt.load('arial.ttf', 16))
	spritesheet = assert(drystal.load_surface('spritesheet.png'))
	drystal.draw_from(spritesheet)
	set_state(states[current_state])

	print('name                max')
end

function drystal.update(dt)
	if tick > 100 then
		state.max = math.max(state.max, number)
		if dt > target * 1.2 then
			number = number - 5
		else
			number = number + 30
		end
	end
	tick = tick + 1
	if tick > 600 and dt - target <= 1 then
		print(state.name .. '        ' .. state.max)
		current_state = current_state + 1
		if states[current_state] then
			tick = 0
			number = 0
			set_state(states[current_state])
		else
			perf_stop()
		end
	end
end

function perf_stop()
	drystal.stop()
end

function drystal.draw()
	state:draw()
end

function drystal.key_press(k)
	if k == 'a' then
		perf_stop()
	end
end
