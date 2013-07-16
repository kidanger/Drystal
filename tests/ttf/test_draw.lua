require 'data/drystal'

function init()
	resize(600, 400)

	set_font('data/arial.ttf', 16)
end

function update_text(text)
	if surf then free_surface(surf) end

	set_color(0,0,0)
	surf = text_surface(text)
	draw_from(surf)
end

function update(dt)
	update_text(tostring(dt))
end

function draw()
	set_color(255,255,255)
	draw_background()

	local w, h = surface_size(surf)
	local sprite = {x=0, y=0, w=w, h=h}
	draw_sprite(sprite, 0, 0)

	flip()
end

function key_press(key)
	engine_stop()
end

