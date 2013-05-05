width = 800
height = 600

speed = 6
ball_speed = 10

state = 'pause'

left = {
    x=0,
    points=0,
    sprite={x=0, y=32, w=4, h=32},
}

right = {
    x=width-4,
    points=0,
    sprite={x=0, y=32, w=4, h=32},
}

function reload_game()
    left.y = height/2-16
    left.dy = 0

    right.y = height/2-16
    right.dy = 0

    ball = {
        x=width / 2,
        y=height / 2,
        dx=ball_speed,
        dy=math.random(-ball_speed/2, ball_speed/2),
        sprite={x=32, y=32, w=13, h=13},
    }
end

function init()
    resize(width, height)
    set_resizable(false)
    show_cursor(false)
    set_background(125, 125, 125)
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
    elseif right.y+right.sprite.h < ball.y then
        right.dy = speed
    else
        right.dy = 0
    end
end

function update_pad(pad)
    pad.y = pad.y + pad.dy
    if pad.y < 0 then
        pad.y = 0
    elseif pad.y+pad.sprite.h > height then
        pad.y = height - pad.sprite.h
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
    if pad.y < y+ball.sprite.h/2 and pad.y+pad.sprite.h > y-ball.sprite.h/2 then
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
end

function draw()
    draw_background()
    draw_sprite(left.sprite, left.x, left.y)
    draw_sprite(right.sprite, right.x, right.y)
    draw_sprite(ball.sprite, ball.x-ball.sprite.w/2, ball.y-ball.sprite.h/2)
    flip()
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
    if key == 'escape' then
        engine_stop()
    end
end

function key_release(key)
    if key == 'up' or key == 'down' then
        left.dy = 0
    end
end
