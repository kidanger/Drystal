local drystal = require 'drystal'

drystal.resize(800, 600)

local gray = false
local multiply = false
local distortion = false
local blur = false
local vignette = false
local pixelate = false

local time = 0
function drystal.update(dt)
	time = time + dt
end

function drystal.draw(dt)
	drystal.set_alpha(255)
	drystal.set_color(255, 255, 255)
	drystal.draw_background()

	drystal.set_color(255, 0, 0)
	drystal.draw_rect(50, 50, 300, 200)

	drystal.set_color(0, 0, 100)
	drystal.set_alpha(200)
	drystal.draw_rect(10, 10, 100, 100)

	drystal.set_color(0, 255, 100)
	drystal.set_alpha(255)
	drystal.draw_rect(100, 100, 100, 100)

	if vignette then
		drystal.postfx('vignette', .7, (math.sin(time)/2 + 0.5) * .2 + .2)
	end
	if multiply then
		drystal.postfx('multiply', 1.9, .8, .7)
	end
	if gray then
		drystal.postfx('gray', math.sin(time)/2 + .5)
	end
	if distortion then
		drystal.postfx('distortion', time, 20, 0)
	end
	if pixelate then
		local s = (math.sin(time / 10) / 2 + .5) * 16
		drystal.postfx('pixelate', s, s)
	end
	if blur then
		drystal.postfx('blur', 90)
	end
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	elseif k == 'g' then
		gray = not gray
	elseif k == 'm' then
		multiply = not multiply
	elseif k == 'd' then
		distortion = not distortion
	elseif k == 'b' then
		blur = not blur
	elseif k == 'v' then
		vignette = not vignette
	elseif k == 'p' then
		pixelate = not pixelate
	end
end
