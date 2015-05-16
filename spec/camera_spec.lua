local drystal = require 'drystal'

describe('camera', function()
	local camera = drystal.camera
	local screen = drystal.screen

	before_each(function()
		pcall(function()
			while true do
				camera.pop()
			end
		end)
		camera.reset()
	end)

	it('initializes the fields', function()
		assert.equals(0, camera.x)
		assert.equals(0, camera.y)
		assert.equals(0, camera.angle)
		assert.equals(1, camera.zoom)
	end)

	it('resets properly', function()
		camera.x = 10
		assert.not_equals(0, camera.x)
		camera.reset()
		assert.equals(0, camera.x)
	end)

	it('can move', function()
		camera.x = 10
		camera.y = 15
		assert.equals(10, camera.x)
		assert.equals(15, camera.y)
	end)

	it('can rotate', function()
		local a = math.pi
		camera.angle = a
		assert.equals(a, camera.angle)
	end)

	it('can zoom', function()
		local z = 1.1
		camera.zoom = z
		assert.equals(z, camera.zoom)
	end)

	it('throws an error when the stack is empty', function()
		assert.error(function()
			camera.pop()
		end)
	end)

	it('throws an error when the stack is full', function()
		assert.error(function()
			while true do
				camera.push()
			end
		end)
	end)

	it('copies values when pushing', function()
		camera.x = 10
		assert.equals(10, camera.x)
		camera.push()
		assert.equals(10, camera.x)
	end)

	it('restores values when popping', function()
		camera.x = 5
		camera.push()
		camera.x = 10
		camera.pop()
		assert.equals(5, camera.x)
	end)

	describe('screen2scene', function()

		it('is correct when simple', function()
			local x, y = drystal.screen2scene(0, 0)
			assert.equals(0, x)
			assert.equals(0, y)

			local x, y = drystal.screen2scene(screen.w, screen.h)
			assert.equals(screen.w, x)
			assert.equals(screen.h, y)
		end)

		-- TODO: moved, rotated, zoomed

	end)

end)

