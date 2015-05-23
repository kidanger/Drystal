drystal = require 'drystal'

describe 'display', ->

	it 'can resize', ->
		drystal.resize 120, 100
		assert.equals 120, drystal.screen.w
		assert.equals 100, drystal.screen.h

		drystal.resize 150, 110
		assert.equals 150, drystal.screen.w
		assert.equals 110, drystal.screen.h

