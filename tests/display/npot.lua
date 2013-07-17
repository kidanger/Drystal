require 'data/drystal'

function init()
	resize(300, 200)
	surf = load_surface('data/npot.png')
	local w, h = surface_size(surf)
	assert(w == 200, 'width is not 200, ' .. w)
	assert(h == 150, 'height is not 150, ' .. h)
	draw_from(surf)
end

function draw()
	local sprite = {x=0,y=0,w=200,h=150}
	draw_sprite(sprite, 0, 0)
	flip()
end

function key_press()
	engine_stop()
end
