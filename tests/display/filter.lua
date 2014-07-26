local drystal = require 'drystal'

local spritesheet = drystal.fromjson(io.open('image.json'):read('*all'))
local image = assert(drystal.load_surface(spritesheet.meta.image))

drystal.resize(600, 400)
image:draw_from()

local spritesrc = spritesheet.frames['character.png'].frame
local sprite = drystal.new_sprite {
	x=300,
	y=180,
	source=spritesrc,
	angle=math.pi/3,
}
local sprite2 = drystal.new_sprite {
	x=220,
	y=180,
	w=spritesrc.w*2,
	h=spritesrc.h*2,
	source=spritesrc,
	angle=math.pi/2,
}
local sprite3 = drystal.new_sprite {
	x=350,
	y=180,
	w=spritesrc.w/2,
	h=spritesrc.h/2,
	source=spritesrc,
	angle=math.pi/2,
}


function drystal.draw()
	drystal.draw_background()

	sprite:draw()
	sprite2:draw()
	sprite3:draw()
	local t = {
		angle=0,
		wfactor=1,
		hfactor=1,
	}
	drystal.draw_sprite({x=spritesrc.x, y=spritesrc.y, w=spritesrc.w, h=spritesrc.h}, 300, 220, t)
end

function drystal.key_press(k)
	if k == '[1]' then
		image:set_filter(drystal.NEAREST)
	elseif k == '[2]' then
		image:set_filter(drystal.LINEAR)
	elseif k == '[3]' then
		image:set_filter(drystal.BILINEAR)
	elseif k == '[4]' then
		image:set_filter(drystal.TRILINEAR)
	elseif k == 'a' then
		drystal.stop()
	end
end

function drystal.mouse_press(x, y, b)
	if b == 4 then
		drystal.camera.zoom = drystal.camera.zoom * 1.2
	elseif b == 5 then
		drystal.camera.zoom = drystal.camera.zoom / 1.2
	end
end
