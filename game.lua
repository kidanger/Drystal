ticks = 0

local mouse_x = 0
local mouse_y = 0
local r, g, b = 125, 0, 0

local sprites = {
    bonhomme={x=0, y=0, w=32, h=32},
    enemy={x=32, y=0, w=32, h=32},
}

local enemy = {
    x=600,
    y=400
}

function init()
    print("initialized from lua")
    show_cursor(false)
    resize(900, 500)
    set_resizable(false)
end

function resize_event(w, h)
    resize(w, h)
end

function update()
    --print("` " .. ticks)
    ticks = ticks + 1
    if mouse_x > enemy.x then
        enemy.x = enemy.x + 10
    else
        enemy.x = enemy.x - 10
    end
    if mouse_y > enemy.y then
        enemy.y = enemy.y + 10
    else
        enemy.y = enemy.y - 10
    end
    r = r + math.random(-5, 5)
    set_background(r, g, b)
end

function mouse_motion(x, y)
    mouse_x = x
    mouse_y = y
end

function mouse_press(x, y, button)
end

function draw()
    draw_background()
    draw_sprite(sprites.bonhomme, mouse_x, mouse_y)
    draw_sprite(sprites.enemy, enemy.x, enemy.y)
    flip()
end

