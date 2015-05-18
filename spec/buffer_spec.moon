drystal = require 'drystal'

describe 'buffer', ->

	before_each ->
		drystal.use_default_buffer!
		drystal.screen\draw_on!
		drystal.set_color 'white'
		drystal.set_alpha 255
		drystal.draw_background!

	it 'resizes', ->
		with drystal.new_buffer 10
			\use!
			drystal.draw_line 0, 0, 1, 1 for i = 0, 50

	it 'should not accept invalid sizes', ->
		assert.error -> drystal.new_buffer -1
		assert.error -> drystal.new_buffer 0

	it 'contains drawing primitives', ->
		assert.color drystal.screen, 1, 1, 'white'
		with drystal.new_buffer!
			\use!
			drystal.set_color 'blue'
			drystal.draw_rect 0, 0, 2, 2

			drystal.use_default_buffer!  -- prevent the buffer from being drawn by screen.draw_on
			assert.color drystal.screen, 1, 1, 'white'
			\draw!
			assert.color drystal.screen, 1, 1, 'blue'

	it 'contains drawing primitives even when upload_and_free-ed', ->
		assert.color drystal.screen, 1, 1, 'white'
		with drystal.new_buffer!
			\use!
			drystal.set_color 'blue'
			drystal.draw_rect 0, 0, 2, 2

			\upload_and_free!
			assert.color drystal.screen, 1, 1, 'white'
			\draw!
			assert.color drystal.screen, 1, 1, 'blue'

	it 'resets correctly', ->
		with drystal.new_buffer!
			\use!
			drystal.set_color 'blue'
			drystal.draw_rect 0, 0, 2, 2

			\reset!
			assert.color drystal.screen, 1, 1, 'white'
			\draw!
			assert.color drystal.screen, 1, 1, 'white'

	it 'cannot contain lines after draw_rect', ->
		with drystal.new_buffer!
			\use!
			drystal.draw_rect 0, 0, 1, 1
			assert.error -> drystal.draw_line 0, 0, 1, 1

	it 'cannot contain points after draw_line', ->
		with drystal.new_buffer!
			\use!
			drystal.draw_line 0, 0, 1, 1
			assert.error -> drystal.draw_point 0, 0, 1

	it 'cannot contain triangles after draw_point', ->
		with drystal.new_buffer!
			\use!
			drystal.draw_point 0, 0, 1
			assert.error -> drystal.draw_rect 0, 0, 1, 1

	it 'cannot contain textured triangle after draw_point', ->
		with buffer, surf = drystal.new_buffer!, drystal.new_surface 1, 1
			buffer\use!
			surf\draw_from!
			drystal.draw_point 0, 0, 1
			assert.error -> drystal.draw_sprite {x: 0, y: 0, w: 1, h:1}, 0, 0

	it 'cannot contain textured points after draw_sprite', ->
		with buffer, surf = drystal.new_buffer!, drystal.new_surface 1, 1
			buffer\use!
			surf\draw_from!
			drystal.draw_sprite {x: 0, y: 0, w: 0, h:0}, 0, 0
			assert.error -> drystal.draw_point 0, 0, 0, 0, 1

	it 'cannot be modified after upload_and_free', ->
		with drystal.new_buffer!
			\upload_and_free!
			assert.error -> \reset!
			assert.error -> \use!
			assert.error -> \upload_and_free!

	it 'should use default buffer after upload_and_free', ->
		with drystal.new_buffer!
			\upload_and_free!
			drystal.set_color 'black'
			drystal.draw_rect 0, 0, 2, 2
			drystal.use_default_buffer!
			assert.color drystal.screen, 1, 1, 'black'

