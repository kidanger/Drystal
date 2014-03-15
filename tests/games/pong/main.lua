local drystal = require "drystal"
local font = require "truetype"
require "draw"

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

local width = 800
local height = 600

local big_font
local normal_font
local small_font

timer = timer or 0
match_duration = 3600
bonus_duration = 360
cheat = false

net_status = {
	hostname = 'localhost',
	port = '1234',
	info = '',
	last_received = '',
}
in_message = false
message = ''

pausestate = {}
endstate = {}
runstate = {}

Pad = Pad or { }
Pad.__index = Pad

function Pad.new(name, posx)
	local pad = {}
	pad.w = 5
	pad.h = 42
	pad.ai = true
	pad.points = 0
	pad.bonus_timer = 0
	pad.posx = posx
	pad.name = name
	return setmetatable(pad, Pad)
end

function Pad:update(dt)
	if cheat and self.bonus_timer == 0 then
		local h = (math.sin(timer / 10)/2+0.5) * 42 + 21
		self:grow_to(h)
	end
	self:update_bonus(dt)
	if self.diry ~= 0 then
		self.dy = math.max(math.min(speed, self.dy + self.diry/4), -speed)
	else
		self.dy = self.dy * 0.7
	end
	self.y = self.y + self.dy * dt
	if self.y < 0 then
		self.y = 0
	elseif self.y+self.h > height then
		self.y = height - self.h
	end
end

function Pad:get_bonus()
	self.bonus_timer = 1
end

function Pad:replace()
	if self.posx < 0 then
		self.x = width + self.posx - self.w
	else
		self.x = self.posx
	end
end
function Pad:update_bonus(dt)
	if self.bonus_timer == 0 then
		return
	end
	self.bonus_timer = self.bonus_timer + 1
	if self.bonus_timer < bonus_duration*0.25 then
		self:grow_to(self.h + 1)
	end
	if self.bonus_timer > bonus_duration*0.75 then
		self:grow_to(self.h - 1)
	end
	if self.bonus_timer > bonus_duration then
		self.bonus_timer = 0
		self:grow_to(42)
	end
end
function Pad:grow_to(h)
	local yy = self.y + self.h / 2
	self.h = h
	self.y = yy - self.h / 2
end

function Pad:get_name()
	return self.ai and 'Computer' or self.name
end
function Pad:reset_intergame()
	self:replace()
	self.y = height/2-16
	self.dy = 0
	self.diry = 0
end
function Pad:reset()
	self:replace()
	self.points = 0
	self.ai = true
	self.h = 42
end

left = left or Pad.new('Player1', 2)
right = right or Pad.new('Player2', -2)

powerup = powerup or {
	x = 0,
	y = 0,
	visible = false,
	sprite = spritesheet.frames['bonus.png'].frame
}

function new_match()
	state = 'run'
	timer = 0
	powerup.visible = false
	right:reset()
	left:reset()
	reload_game()
end

function reload_game()
	setup_speeds()

	left:reset_intergame()
	right:reset_intergame()

	ball = {
		x=width / 2,
		y=height / 2,
		radius=6,
		dx=ball_speed * (left.points < right.points and 1 or -1),
		dy=math.random(-ball_speed/4, ball_speed/4),
	}
end

function drystal.init()
	drystal.resize(width, height)
	drystal.show_cursor(false)

	big_font = font.load('arial.ttf', 16)
	normal_font = font.load('arial.ttf', 14)
	small_font = font.load('arial.ttf', 12)

	drystal.draw_from(drystal.load_surface('image.png'))
	if not state then
		new_match()
		state = 'pause'
	end
	print('Press ENTER to start the game.')
	print('Player1 uses Z and S keys.')
	print('Player2 uses Up and Down arrow keys.')
end

function setup_speeds()
	ball_speed = 360 + 100*width/800
	speed = 300 + 50*height/600
end

function left_loose()
	state = 'pause'
	right.points = right.points + 1
end

function right_loose()
	state = 'pause'
	left.points = left.points + 1
end

function drystal.update(dt)
	if state == 'run' then
		if right.ai then
			update_ai(right, dt)
		end
		if left.ai then
			update_ai(left, dt)
		end

		left:update(dt)
		right:update(dt)
		update_ball(dt, ball)

		update_powerup()

		update_timer(dt)

		if ball.x <= ball.radius then
			check_collision(left, left_loose)
		end
		if ball.x >= width - ball.radius then
			check_collision(right, right_loose)
		end
	end
end

function update_ai(pad, dt)
	dt = dt * 2
	if pad.y > ball.y+ball.dy*dt then
		pad.diry = -speed
	elseif pad.y+pad.h < ball.y+ball.dy*dt then
		pad.diry = speed
	else
		pad.diry = 0
	end
end

function update_powerup()
	if not powerup.visible and timer > 120 and math.random() < 0.01 then
		powerup.x = math.random(width*0.25, width*0.75)
		powerup.y = math.random(32, height-32*2)
		powerup.visible = true
	end
end

function update_ball(dt, ball)
	ball.x = ball.x + ball.dx * dt
	ball.y = ball.y + ball.dy * dt
	if ball.y <= 0 or ball.y+ball.radius*2 >= height then
		ball.dy = ball.dy * -1
	end
	min_dist = math.min(width - ball.x, ball.x)
	ball.radius = 6 + min_dist*5 / width

	local sprite = powerup.sprite
	if powerup.visible and ball.x > powerup.x and ball.x < powerup.x+sprite.w
		and ball.y > powerup.y and ball.y < powerup.y+sprite.h then
		if ball.dx < 0 then -- right pad is the last which hit the ball
			right:get_bonus()
		else
			left:get_bonus()
		end
		powerup.visible = false
	end
end

function update_timer(dt)
	timer = timer + dt * 50
	if timer > match_duration then
		end_match()
	end
end

function check_collision(pad, callback)
	local y = ball.y
	if pad.y < y+ball.radius and pad.y+pad.h > y-ball.radius then
		ball.dx = ball.dx * -1.001
		ball.dy = ball.dy * 0.9 + pad.dy * 0.2
	else
		callback()
	end
end

function end_match()
	state = 'end'
end

ticks = 0
function drystal.draw()
	ticks = ticks + 1
	drystal.set_color(GRAY)
	drystal.set_alpha(255)
	drystal.draw_background()

	local score = left:get_name():upper() .. ' ' .. left.points .. ' - ' .. right.points .. ' ' .. right:get_name():upper()
	font.use(normal_font)
	drystal.set_color(BLACK)
	font.draw(score, (width - #score * 8) / 2, 50)

	local barw = 200
	bar = progress(width/2 - barw/2, 25, barw, 20)
	bar.border_color = BLACK
	bar.progress_color = BLACK
	if timer >= match_duration * 0.75 and math.floor(timer/20) % 2 == 0 then
		bar.progress_color = RED
	end
	bar.type = BAR
	bar.ratio = timer / match_duration
	bar.draw()

	drystal.set_color(BLACK)
	drystal.draw_rect(left.x, left.y, left.w, left.h)
	drystal.draw_rect(right.x, right.y, right.w, right.h)

	if cheat then
		drystal.draw_line(ball.x, ball.y, ball.x+ball.dx*10, ball.y+ball.dy*10)
	end

	drystal.draw_circle(ball.x, ball.y, ball.radius)

	if powerup.visible then
		drystal.set_color(WHITE)
		drystal.draw_sprite(powerup.sprite, powerup.x, powerup.y)
	end

	if state == 'pause' then
		font.use(big_font)
		local message = 'PRESS ENTER'
		local sx, sy = font.sizeof(message)
		local x, y = (width - sx) / 2, (height - sy) / 2
		draw_frame(x, y, sx + 10, sy + 10, BLACK, DARK_GRAY, 2)
		drystal.set_color({(math.cos(ticks/10)/2+0.5)*150 + 100, 50, 50})
		font.draw(message, x+5, y+5)
	end

	if state == 'end' then
		---
		local sx = width / 3
		local sy = 16 * 3
		local x, y = (width - sx) / 2, (height - sy) / 2
		draw_frame(x, y, sx + 10, sy + 10, BLACK, DARK_GRAY, 3)
		drystal.set_color(BLACK)
		---
		local message = "Time's up"
		local sx, sy = font.sizeof(message)
		local x, y = (width - sx) / 2, y + 5
		font.draw(message, x+5, y+5)
		---
		local message = (left.points > right.points and left:get_name() or right:get_name()) .. ' won.'
		local sx, sy = font.sizeof(message)
		local x, y = (width - sx) / 2, y + sy + 2
		font.draw(message, x+5, y+5)
		---
	end

	if in_message then
		draw_say()
	else
		draw_net_status()
	end
end

function draw_say()
	drystal.set_color(RED)
	font.use(small_font)
	local sx, sy = font.sizeof(message)
	local x, y = 1, height - sy - 1
	font.draw(message, x, y)
end

function draw_net_status()
	drystal.set_color(BLACK)
	font.use(small_font)
	local str = net_status.info
	local sx, sy = font.sizeof(str)
	local x, y = 1, height - sy*2 - 2
	font.draw(str, x, y)
	font.draw(net_status.last_received, x, y+sy+1)
end

function drystal.key_press(key)
	if in_message then
		if key == 'escape' then
			in_message = false
		end
		if key == 'return' then
			in_message = false
			if message == 'imanoob' then
				cheat = true
			end
		elseif key == 'space' then
			message = message .. ' '
		elseif key == 'backspace' then
			message = message:sub(1, #message - 1)
		elseif #key == 1 then
			message = message .. key
		end
		return
	end
	if state == 'run' then
		if key == 'up' then
			right.ai = false
			right.diry = -speed
		end
		if key == 'down' then
			right.ai = false
			right.diry = speed
		end
		if key == 'z' then
			left.ai = false
			left.diry = -speed
		end
		if key == 's' then
			left.ai = false
			left.diry = speed
		end
	end
	if state == 'pause' then
		if key == 'return' then
			state = 'run'
			reload_game()
		end
	end
	if state == 'end' then
		if key == 'return' then
			new_match()
		end
	end
	if key == 't' then
		in_message = true
		message = ''
	end
	if key == 'escape' then
		drystal.engine_stop()
	end
end

function drystal.key_release(key)
	if key == 'up' or key == 'down' then
		right.diry = 0
	end
	if key == 'z' or key == 's' then
		left.diry = 0
	end
end

