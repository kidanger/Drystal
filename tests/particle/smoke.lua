local drystal = require 'drystal'

tex = drystal.load_surface('spritesheet.png')
local sys_smoke = drystal.new_system(200, 596)

sys_smoke:set_sizes {
	[0]=100,
	[0.25] = 75,
	[0.5] = 55,
	[1] = 25,
}
sys_smoke:set_colors {
	[0]=drystal.colors.white,
	[0.4]=drystal.colors.gray,
	[0.7]=drystal.colors.gray:darker(),
	[1]='black',
}

sys_smoke:set_lifetime(3)
sys_smoke:set_direction(-math.pi / 2 - math.pi/8, -math.pi/2 + math.pi/8)
sys_smoke:set_initial_velocity(100, 150)
sys_smoke:set_initial_acceleration(0)
sys_smoke:set_offset(-10, 20, 0, 10)
sys_smoke:set_emission_rate(16)
sys_smoke:set_texture(tex, 0, 0)

local sys_smoke_opaque = drystal.new_system(600, 600)

sys_smoke_opaque:set_sizes {
	[0]=100,
	[0.25] = 75,
	[0.5] = 55,
	[1] = 25,
}
sys_smoke_opaque:set_colors {
	[0]=drystal.colors.white,
	[0.4]=drystal.colors.white:darker(),
	[0.9]='black',
}

sys_smoke_opaque:set_lifetime(3)
sys_smoke_opaque:set_direction(-math.pi / 2 - math.pi/8, -math.pi/2 + math.pi/8)
sys_smoke_opaque:set_initial_velocity(100, 150)
sys_smoke_opaque:set_initial_acceleration(0)
sys_smoke_opaque:set_emission_rate(25)
sys_smoke_opaque:set_texture(tex, 0, 0)

function drystal.init()
	drystal.resize(800, 600)
	sys_smoke:start()
	sys_smoke_opaque:start()
end

function drystal.update(dt)
	if dt > .06 then
		dt = .06
	end
	sys_smoke:update(dt)
	sys_smoke_opaque:update(dt)
end

function drystal.draw()
	drystal.set_blend_mode(drystal.blends.default)
	drystal.set_color('black')
	drystal.set_alpha(255)
	drystal.draw_background()

	sys_smoke_opaque:draw()

	drystal.set_alpha(75)
	drystal.set_blend_mode(drystal.blends.add)
	sys_smoke:draw()
end

function drystal.key_press(k)
	if k == 'a' then
		drystal.stop()
	end
end

