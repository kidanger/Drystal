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

local gamestate = {
    previous = nil,

    draw = function()
        draw_background()
        draw_sprite(sprites.bonhomme, mouse_x, mouse_y)
        draw_sprite(sprites.enemy, enemy.x, enemy.y)
        flip()
    end,

    update = function()
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
    end,

    mouse_motion = function (x, y)
        mouse_x = x
        mouse_y = y
    end,

    mouse_press = function(x, y, b)
        if b == 3 then
            pop_state()
        end
    end,
}

local pausestate = {
    previous = nil,

    draw = function()
        draw_background()
        draw_sprite(sprites.bonhomme, mouse_x, mouse_y)
        draw_sprite(sprites.enemy, enemy.x, enemy.y)
        flip()
    end,

    mouse_press = function(x, y, b)
        if b == 1 then
            push_state(gamestate)
        end
    end,

    on_enter = function()
        set_background(0, 0, 0)
    end
}

local state = nil

function push_state(new_state)
    new_state.previous = state
    change_state(new_state)
end

function change_state(new_state)
    if state ~= nil and state.on_exit ~= nil then
        state.on_exit()
    end
    state = new_state
    if state.on_enter ~= nil then
        state.on_enter()
    end

    -- set callbacks
    draw = state.draw
    update = state.update
    mouse_press = state.mouse_press
    mouse_motion = state.mouse_motion
end

function pop_state()
    if state.previous == nil then
        print ("no state to pop")
    else
        change_state(state.previous)
    end
end

function init()
    print("initialized from lua")
    show_cursor(false)
    resize(900, 500)
    set_resizable(true)

    push_state(pausestate)
end

function resize_event(w, h)
    resize(w, h)
end

