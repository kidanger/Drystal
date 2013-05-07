require "data/draw"

width = 800
height = 600
fill = true

speed = 6
ball_speed = 10

state = 'pause'

background = {
	r = 125,
	g = 125,
	b = 125,
}

left = {
	x = 2,
	w = 5,
	h = 32,
	points = 0,
}

right = {
	x = width-6,
	w = 5,
	h = 32,
	points = 0,
}

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
	reload_game()
end

function left_loose()
	print ("Player loses!")
	state = 'pause'
	right.points = right.points + 1
	print ("Score: " .. right.points .. " | " .. left.points)
end

function right_loose()
	print ("AI loses!")
	state = 'pause'
	left.points = left.points + 1
	print ("Score: " .. right.points .. " | " .. left.points)
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

function update()
	if state == 'run' then
		update_ai()

		update_pad(left)
		update_pad(right)
		update_ball(ball)

		if ball.x <= ball.sprite.w/2 then
			check_collision(left, left_loose)
		end
		if ball.x >= width - ball.sprite.w/2 then
			check_collision(right, right_loose)
		end
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

function update_ball(ball)
	ball.x = ball.x + ball.dx
	ball.y = ball.y + ball.dy
	if ball.y <= 0 or ball.y+ball.sprite.h >= height then
		ball.dy = ball.dy * -1
	end
	min_dist = math.min(width - ball.x, ball.x)
	ball.radius = 6 + min_dist*10 / width
end

font_size = 1
ticks = 0
function draw()
	ticks = ticks + 0.1
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

	a = (math.cos(ticks/10+math.pi)/2 + 0.5) * 360 - 90
	al = (math.cos(ticks/10+math.pi)/2 + 0.5) * 255
	set_alpha(al)
	draw_arc(40, 40, 20, -90, a)
	set_alpha(255)

	if state == 'pause' then
		set_font("data/arial.ttf", font_size)
		font_size = 16
		local message = 'PRESS ENTER'
		local sx, sy = text_size(message)
		set_offset((width - sx) / 2, (height - sy) / 2)
		set_round(font_size/5)
		draw_frame(sx + 10, sy + 10, BLACK, DARK_GRAY, 3)
		set_color({(math.cos(ticks)/2+0.5)*255, 50, 50})
		set_round(0)
		draw_text(message, 5, 5)
		set_offset(0, 0)
	end

	--stress()
	flip()
end

function stress()
	for i = 0, 200 do
		set_round(math.random(0, 1))
		set_font("data/arial.ttf", font_size)
		font_size = math.random(10, 30)
		local message = tostring(math.random())
		local sx, sy = text_size(message)
		local x = math.random(0, width-font_size)
		local y = math.random(0, height-font_size*#message)
		set_offset(x, y)
		draw_frame(sx + 10, sy + 10, BLACK, DARK_GRAY, 5)
		set_color({font_size*3, 50, 50})
		draw_text(message, 5, 5)
		set_offset(0, 0)
	end
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
		if key == 'return' then
			state = 'run'
			reload_game()
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

