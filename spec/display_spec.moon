drystal = require 'drystal'

describe 'display', ->

	before_each ->
		drystal.set_alpha 255
		drystal.set_color 'black'
		drystal.screen\draw_on!
		drystal.draw_background!

	it 'can resize', ->
		drystal.resize 120, 100
		assert.equals 120, drystal.screen.w
		assert.equals 100, drystal.screen.h

		drystal.resize 150, 110
		assert.equals 150, drystal.screen.w
		assert.equals 110, drystal.screen.h

	it 'draws lines', ->
		drystal.set_color 'red'
		drystal.draw_line 10, 10, 20, 10
		for i = 11, 20
			assert.color drystal.screen, i, 10, 'red'
			assert.color drystal.screen, i, 11, 'black'

