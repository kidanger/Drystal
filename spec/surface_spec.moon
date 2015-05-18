drystal = require 'drystal'

describe 'surface', ->
	
	before_each ->
		drystal.set_alpha 255
		drystal.set_color 'black'

	it 'can be created', ->
		drystal.new_surface 10, 10
		drystal.new_surface 10, 10, true

	it 'has the correct default color/alpha', ->
		with surf = drystal.new_surface 10, 10
			assert.color surf, 1, 1, 'black', 0

		drystal.set_color 'red'
		drystal.set_alpha 120
		with surf = drystal.new_surface 10, 10
			assert.color surf, 1, 1, 'black', 0

	describe 'size', ->

		it 'should be positive', ->
			drystal.new_surface 1, 1
			assert.error -> drystal.new_surface 0, 0
			assert.error -> drystal.new_surface -1, 1
			assert.error -> drystal.new_surface 1, -1

		it 'should be equal or smaller than 2048x2048', ->
			drystal.new_surface 2048, 2048
			assert.error -> drystal.new_surface 2049, 2048
			assert.error -> drystal.new_surface 2048, 2049

		it 'should be correct', ->
			with drystal.new_surface 40, 30
				assert.equals 40, .w
				assert.equals 30, .h
			with drystal.new_surface 40, 30, true
				assert.equals 40, .w
				assert.equals 30, .h

	describe 'get_pixel', ->

		it 'throws an error if the position is invalid', ->
			with drystal.new_surface 10, 10
				\get_pixel 1, 1
				\get_pixel .w, 1
				\get_pixel 1, .h
				assert.error -> \get_pixel 0, 1
				assert.error -> \get_pixel 1, 0
				assert.error -> \get_pixel .w + 1, 1
				assert.error -> \get_pixel 1, .h + 1

		it 'throws and error if the surface is being drawn on', ->
			with drystal.new_surface 10, 10
				\get_pixel 1, 1
				\draw_on!
				assert.error -> \get_pixel 1, 1

		it 'works after a draw_background', ->
			with surf = drystal.new_surface 10, 10
				assert.color surf, 1, 1, 'black', 0
				drystal.set_color 'red'
				\draw_on!
				drystal.draw_background!
				drystal.screen\draw_on!
				assert.color surf, 1, 1, 'red'

	describe 'set_filter', ->

		it 'throws an error if the filter is invalid', ->
			with drystal.new_surface 10, 10
				\set_filter drystal.filters.nearest
				\set_filter drystal.filters.linear
				\set_filter drystal.filters.bilinear
				\set_filter drystal.filters.trilinear
				assert.error -> \set_filter -1

		it 'throws an error if the filter is invalid when npot', ->
			with drystal.new_surface 10, 10, true
				\set_filter drystal.filters.nearest
				\set_filter drystal.filters.linear
				assert.error -> \set_filter drystal.filters.bilinear
				assert.error -> \set_filter drystal.filters.trilinear
				assert.error -> \set_filter -1

-- TODO: load, draw_on, draw_from

