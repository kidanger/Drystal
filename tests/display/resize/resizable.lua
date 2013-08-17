require 'drystal'
local tt = require 'truetype'

w, h = 600, 400
text = ''

function init()
    resize(w, h)
    tt.use(tt.load('arial.ttf', 20))
    update_surf()
end

function draw()
    set_color(255, 255, 255)
    draw_background()

    set_color(0, 0, 0)
    tt.draw(text, 0, 0)

    flip()
end

function key_press(key)
    if key == 'h' then
        w = w - 100
    elseif key == 'l' then
        w = w + 100
    elseif key == 'j' then
        h = h + 100
    elseif key == 'k' then
        h = h - 100
    else
        return
    end
    resize(w, h)
    update_surf()
end

function update_surf()
    print("screen size", surface_size(screen))
    text = "hjlk to resize, current size is " .. w .. "x" .. h
end

function resize_event(w, h)
    print("should not be printed")
end
