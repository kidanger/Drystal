
RED = { 230, 30, 30 }
BLACK = { 20, 20, 20 }
DARK_GRAY = { 90, 90, 90 }
GRAY = { 150, 150, 150 }


function draw_frame(x, y, w, h, border_color, inside_color, border_size)
    set_color(border_color)
    draw_rect(x, y, w, h)
    set_color(inside_color)
    draw_rect(x+border_size, y+border_size, w-border_size*2, h-border_size*2)
end

