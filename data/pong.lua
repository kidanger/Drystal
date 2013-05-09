require "data/draw"

width = 800
height = 600
fill = true

speed = 6
ball_speed = 10

timer = 0
match_duration = 3600

net_status = {
	status = '',
	last_received = '',
}

background = {
	r = 125,
	g = 125,
	b = 125,
}

left = {
	x = 2,
	w = 5,
	h = 32,
}

right = {
	x = width-7,
	w = 5,
	h = 32,
}

function new_match()
	state = 'run'
	timer = 0
	left.points = 0
	right.points = 0
	reload_game()
end

function reload_game()
	left.y = height/2-16
	left.dy = 0

	right.y = height/2-16
	right.dy = 0

	ball = {
		x=width / 2,
		y=height / 2,
		radius=6,
		dx=ball_speed,
		dy=math.random(-ball_speed/2, ball_speed/2),
		sprite={x=0, y=32, w=13, h=13},
	}
end

function init()
	resize(width, height)
	set_resizable(false)
	show_cursor(false)

	set_font('data/arial.ttf', 14)
	new_match()
	state = 'pause'
	print(connect('localhost', 1234))
	net_status.info = 'connecting'
end

function left_loose()
	state = 'pause'
	right.points = right.points + 1
end

function right_loose()
	state = 'pause'
	left.points = left.points + 1
end

function update()
	if state == 'run' then
		if net_status.info ~= 'connected' then
			update_ai()
		end

		update_pad(left)
		update_pad(right)
		update_ball(ball)

		update_timer()

		if ball.x <= ball.sprite.w/2 then
			check_collision(left, left_loose)
		end
		if ball.x >= width - ball.sprite.w/2 then
			check_collision(right, right_loose)
		end
	end
end

function update_ai()
	if ball.dx < 0 then
		right.dy = 0
		return
	end
	if right.y > ball.y then
		right.dy = -speed
	elseif right.y+right.h < ball.y then
		right.dy = speed
	else
		right.dy = 0
	end
end

function update_pad(pad)
	pad.y = pad.y + pad.dy
	if pad.y < 0 then
		pad.y = 0
	elseif pad.y+pad.h > height then
		pad.y = height - pad.h
	end
end

function update_ball(ball)
	ball.x = ball.x + ball.dx
	ball.y = ball.y + ball.dy
	if ball.y <= 0 or ball.y+ball.sprite.h >= height then
		ball.dy = ball.dy * -1
	end
	min_dist = math.min(width - ball.x, ball.x)
	ball.radius = 6 + min_dist*10 / width
end

function update_timer()
	timer = timer + 1
	if timer > match_duration then
		end_match()
	end
end

function check_collision(pad, callback)
	local y = ball.y
	if pad.y < y+ball.sprite.h/2 and pad.y+pad.h > y-ball.sprite.h/2 then
		ball.dx = ball.dx * -1.03
		ball.dy = ball.dy + math.random(-1, 1)
	else
		callback()
	end
end

function end_match()
	state = 'end'
end

font_size = 1
ticks = 0
function draw()
	ticks = ticks + 1
	set_color(GRAY)
	draw_background()

	set_color(BLACK)
	draw_rect(left.x, left.y, left.w, left.h)
	draw_rect(right.x, right.y, right.w, right.h)

	draw_circle(ball.x, ball.y, ball.radius)

	local score = 'PLAYER ' .. left.points .. ' ' .. right.points .. ' COMPUTER'
	set_color(BLACK)
	set_font("data/arial.ttf", 14)
	draw_text(score, (width - #score * 8) / 2, 50)

	if timer >= match_duration * 0.75 and math.floor(timer/20) % 2 == 0 then
		set_color(RED)
	end
	a = timer / match_duration * 360 - 90
	set_fill(false)
	draw_circle(width/2, 25, 20)
	set_fill(true)
	draw_arc(width/2, 25, 20, -90, a)
	set_color(BLACK)

	if state == 'pause' then
		local font_size = 16
		set_font("data/arial.ttf", font_size)
		local message = 'PRESS ENTER'
		local sx, sy = text_size(message)
		local x, y = (width - sx) / 2, (height - sy) / 2
		set_round(font_size/5)
		draw_frame(x, y, sx + 10, sy + 10, BLACK, DARK_GRAY, 3)
		set_color({(math.cos(ticks/10)/2+0.5)*255, 50, 50})
		draw_text(message, x+5, y+5)
		set_round(0)
	end

	if state == 'end' then
		set_font("data/arial.ttf", 16)
		set_round(3)
		---
		local sx = width / 3
		local sy = 16 * 3
		local x, y = (width - sx) / 2, (height - sy) / 2
		draw_frame(x, y, sx + 10, sy + 10, BLACK, DARK_GRAY, 3)
		set_color(BLACK)
		---
		local message = "Time's up"
		local sx, sy = text_size(message)
		local x, y = (width - sx) / 2, y + 5
		draw_text(message, x+5, y+5)
		---
		local message = left.points > right.points and "Player won !" or 'Computer won.'
		local sx, sy = text_size(message)
		local x, y = (width - sx) / 2, y + sy + 2
		draw_text(message, x+5, y+5)
		---
		set_round(0)
	end

	draw_net_status()

	--stress()
	flip()
end

function set_font_size(size)
	set_font('data/arial.ttf', size)
end

function draw_net_status()
	set_font_size(11)
	set_color(BLACK)
	local str = net_status.info
	local sx, sy = text_size(str)
	local x, y = 1, height - sy*2 - 2
	draw_text(str, x, y)
	draw_text(net_status.last_received, x, y+sy+1)
end

function key_press(key)
	if state == 'run' then
		if key == 'up' then
			left.dy = -speed
		end
		if key == 'down' then
			left.dy = speed
		end
	end
	if state == 'pause' then
		send('unpause', #'unpause')
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
	if key == 'f' then
		fill = not fill
		set_fill(fill)
	end
	if key == 'escape' then
		engine_stop()
	end
end

function key_release(key)
	if key == 'up' or key == 'down' then
		left.dy = 0
	end
end

function receive(str)
	print('-> ' .. str)
	send(str, #str)
	net_status.last_received = str
	if state == 'run' then
		if str == 'up' then
			right.dy = -speed
		end
		if str == 'down' then
			right.dy = speed
		end
		if str == 'stop' then
			right.dy = 0
		end
	end
end

function connected()
	print('connected to localhost !')
	net_status.info = 'connected'
end
function disconnected()
	print('disconnected from the server !')
	net_status.info = 'disconnected'
end

