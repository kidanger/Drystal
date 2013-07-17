require 'data/drystal'

function init()
	resize(600, 400)
end

function update()
end

function draw_text(text, textcolor, fontname, size, y, drawcolor)
	drawcolor = drawcolor or {255,255,255}

	set_font(fontname, size)
	set_color(textcolor)
	surf = text_surface(text)

	local w, h = surface_size(surf)
	local sprite = {x=0, y=0, w=w, h=h}

	set_color(drawcolor)
	draw_from(surf)
	draw_sprite(sprite, 0, y)

	free_surface(surf)
	return y+h
end


function draw()
	set_color(255,255,255)
	draw_background()

	local y = 0
	y = draw_text('Red Arial 16', {255,0,0}, 'data/arial.ttf', 16, y, {255,255,255})
	y = draw_text('Black Arial 20', {0,0,0}, 'data/arial.ttf', 20, y, {255,255,255})
	y = draw_text('Black Arial 24', {255,255,255}, 'data/arial.ttf', 24, y, {0,0,0})

	flip()
end

function key_press(key)
	engine_stop()
end

