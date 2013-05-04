

ticks = 0
local mouse_x = 0
local mouse_y = 0
local bonbons = 0
local delta_bonbons = 0

function init()
    print("initialized from lua")
end

function update()
    --print("` " .. ticks)
    --print (mouse_x .. " " .. mouse_y)
    ticks = ticks + 1
    if ticks % 20 == 0 and delta_bonbons > 0 then
        bonbons = bonbons + delta_bonbons
        print ("Vous avez ramasser " .. delta_bonbons .. " bonbons !")
        print ("Vous en avez maintenant " .. bonbons .. ".")
        delta_bonbons = 0
        if math.random(0, 100) > 90 then
            print ("Pensez à aller voir le marchant ^^")
        end
    end
end

function mouse_motion(x, y)
    mouse_x = x
    mouse_y = y
end

function mouse_press(x, y, button)
    delta_bonbons = delta_bonbons + 1
end

function draw()
    -- draw_sprite(x, y)
end

