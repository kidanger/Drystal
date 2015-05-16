drystal = require 'drystal'

describe 'camera', ->
	camera = drystal.camera
	screen = drystal.screen

	before_each ->
		pcall ->
			while true
				camera.pop!
		camera.reset!

	it 'initializes the fields', ->
		assert.equals 0, camera.x
		assert.equals 0, camera.y
		assert.equals 0, camera.angle
		assert.equals 1, camera.zoom

	it 'resets properly', ->
		camera.x = 10
		assert.not_equals 0, camera.x
		camera.reset!
		assert.equals 0, camera.x

	it 'can move', ->
		camera.x = 10
		camera.y = 15
		assert.equals 10, camera.x
		assert.equals 15, camera.y

	it 'can rotate', ->
		a = math.pi
		camera.angle = a
		assert.equals a, camera.angle

	it 'can zoom', ->
		z = 1.1
		camera.zoom = z
		assert.equals z, camera.zoom

	it 'throws an error when the stack is empty', ->
		assert.error -> camera.pop!

	it 'throws an error when the stack is full', ->
		assert.error ->
			while true
				camera.push!

	it 'copies values when pushing', ->
		camera.x = 10
		assert.equals 10, camera.x
		camera.push!
		assert.equals 10, camera.x

	it 'restores values when popping', ->
		camera.x = 5
		camera.push!
		camera.x = 10
		camera.pop!
		assert.equals 5, camera.x

	describe 'screen2scene', ->

		it 'is correct when simple', ->
			x, y = drystal.screen2scene 0, 0 
			assert.equals 0, x
			assert.equals 0, y

			x, y = drystal.screen2scene screen.w, screen.h
			assert.equals screen.w, x
			assert.equals screen.h, y

		-- TODO: moved, rotated, zoomed

