local machine = require "data/state"
local r, g, b = 125, 0, 0

local sprites = {
	bonhomme={x=0, y=0, w=32, h=32},
	enemy={x=32, y=0, w=32, h=32},
}

local me = {
	x=0,
	y=0,
	dx = 0,
	dy = 0,
}
local enemy = {
	x=600,
	y=400,
}

local gamestate = {
	on_enter = function()
		show_cursor(false)
	end,

	draw = function()
		draw_background()
		draw_sprite(sprites.bonhomme, me.x, me.y)
		draw_sprite(sprites.enemy, enemy.x, enemy.y)
	end,

	update = function()
		if me.x > enemy.x then
			enemy.x = enemy.x + 10
		else
			enemy.x = enemy.x - 10
		end
		if me.y > enemy.y then
			enemy.y = enemy.y + 10
		else
			enemy.y = enemy.y - 10
		end
		me.x = me.x + me.dx * 5
		me.y = me.y + me.dy * 5
		r = r + math.random(-5, 5)
		set_color({r, g, b})
	end,

	mouse_press = function(x, y, b)
		if b == 3 then
			machine:pop()
		end
	end,

	key_press = function(key)
		if key == 'right' then
			me.dx = me.dx + 1
		end
		if key == 'left' then
			me.dx = me.dx - 1
		end
		if key == 'up' then
			me.dy = me.dy - 1
		end
		if key == 'down' then
			me.dy = me.dy + 1
		end
	end,
	key_release = function(key)
		if key == 'right' then
			me.dx = me.dx - 1
		end
		if key == 'left' then
			me.dx = me.dx + 1
		end
		if key == 'up' then
			me.dy = me.dy + 1
		end
		if key == 'down' then
			me.dy = me.dy - 1
		end
	end
}

local pausestate = {
	draw = function()
		draw_background()
		draw_sprite(sprites.bonhomme, me.x, me.y)
		draw_sprite(sprites.enemy, enemy.x, enemy.y)
	end,

	mouse_press = function(x, y, b)
		if b == 1 then
			machine:push(gamestate)
		end
	end,

	on_enter = function()
		set_color({0, 0, 0})
		show_cursor(true)
	end,
}

function draw()
	machine:call('draw')
	flip()
end
function update()
	machine:call('update')
end
function mouse_press(x, y, button)
	machine:call('mouse_press', x, y, button)
end
function mouse_motion(x, y)
	machine:call('mouse_motion', x, y)
end
function key_press(key)
	if key == 'escape' then
		machine:pop()
	else
		machine:call('key_press', key)
	end
end
function key_release(key)
	machine:call('key_release', key)
end

function init()
	print("initialized from lua")
	resize(900, 500)
	set_resizable(false)

	machine:push(pausestate)
end

function resize_event(w, h)
	resize(w, h)
end
