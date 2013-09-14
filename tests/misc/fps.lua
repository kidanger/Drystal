local drystal = drystal

function drystal.init()
	drystal.resize(400, 400)
end

function drystal.update(dt)
	print(1000/dt, dt)
end

function drystal.key_press(key)
	if key == 'a' then
		drystal.stop()
	end
end

