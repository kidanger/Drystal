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
        flip()
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
        set_background(r, g, b)
    end,

    mouse_motion = function (x, y)
    end,

    mouse_press = function(x, y, b)
        if b == 3 then
            pop_state()
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
        flip()
    end,

    mouse_press = function(x, y, b)
        if b == 1 then
            push_state(gamestate)
        end
    end,

    on_enter = function()
        set_background(0, 0, 0)
        show_cursor(true)
    end,
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
    key_release = state.key_release
    key_press = function(key)
        if key == 'escape' then
            pop_state()
        else
            if state.key_press ~= nil then
                state.key_press(key)
            end
        end
    end
end

function pop_state()
    if state.previous == nil then
        print ("no state to pop, stop engine")
        engine_stop()
    else
        change_state(state.previous)
    end
end

function init()
    print("initialized from lua")
    resize(900, 500)
    set_resizable(false)

    push_state(pausestate)
end

function resize_event(w, h)
    resize(w, h)
end
