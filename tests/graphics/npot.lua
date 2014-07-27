local drystal = require 'drystal'

function drystal.init()
	drystal.resize(300, 200)
	surf = assert(drystal.load_surface('npot.png'))
	local w, h = surf.w, surf.h

	assert(w == 200, 'width is not 200, ' .. w)
	assert(h == 150, 'height is not 150, ' .. h)
	surf:draw_from()
end

function drystal.draw()
	local sprite = {x=0,y=0,w=200,h=150}
	drystal.draw_sprite(sprite, 0, 0)
end

function drystal.key_press()
	drystal.stop()
end
