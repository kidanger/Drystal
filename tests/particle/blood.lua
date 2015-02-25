local drystal = require 'drystal'

local squish = assert(drystal.load_sound('squish2.wav'))
tex = assert(drystal.load_surface('blood.png'))
local sys_blood = drystal.new_system(300, 600)

sys_blood:set_sizes {
	[0]=50,
	[0.25] = 75,
	[0.5] = 55,
	[0.75] = 25,
	[1] = 15,
}
sys_blood:set_colors {
	[0]=drystal.colors.white,
}

sys_blood:set_lifetime(0.5)
sys_blood:set_direction(-math.pi, math.pi)
sys_blood:set_initial_velocity(50, 150)
sys_blood:set_initial_acceleration(-100, 0)
sys_blood:set_offset(0, 0, 0, 0)
sys_blood:set_texture(tex)

function drystal.init()
	drystal.resize(800, 600)
end

local time = 0
function drystal.update(dt)
	time = time + dt
	sys_blood:update(dt)
end

function drystal.draw()
	drystal.set_color('black')
	drystal.draw_background()

	sys_blood:draw()
end

local last_splash = 0
function drystal.mouse_press(x, y, b)
	if time - last_splash > 0.03 then
		sys_blood:emit(60)
		squish:play()
	end
	last_splash = time
end

function drystal.mouse_motion(x, y)
	sys_blood:set_position(x, y)
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end
