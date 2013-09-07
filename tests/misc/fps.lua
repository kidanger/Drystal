local drystal = drystal

function init()
	drystal.resize(400, 400)
end

function update(dt)
	print(1000/dt, dt)
end

function key_press(key)
	if key == 'a' then
		drystal.stop()
	end
end

