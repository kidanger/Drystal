local drystal = require 'drystal'

function drystal.init()
	drystal.resize(300, 200)
	surf = drystal.load_surface('npot.png')
	local w, h = drystal.surface_size(surf)
	assert(w == 200, 'width is not 200, ' .. w)
	assert(h == 150, 'height is not 150, ' .. h)
	drystal.draw_from(surf)
end

function drystal.draw()
	local sprite = {x=0,y=0,w=200,h=150}
	drystal.draw_sprite(sprite, 0, 0)
end

function drystal.key_press()
	drystal.stop()
end
