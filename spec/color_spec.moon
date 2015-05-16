drystal = require 'drystal'

describe 'color', ->

	it 'has W3C colors', ->
		assert.same {255, 0, 0}, {table.unpack drystal.colors.red}
		assert.same {0, 0, 0}, {table.unpack drystal.colors.black}

	it 'supports hexadecimal formatting', ->
		assert.same {0, 0, 0}, {table.unpack drystal.colors['#000000']}
		assert.same {255, 255, 255}, {table.unpack drystal.colors['#ffffff']}
		assert.same {15, 15, 15}, {table.unpack drystal.colors['#0f0f0f']}
		assert.same {16, 16, 16}, {table.unpack drystal.colors['#101010']}
		assert.same {255, 255, 255}, {table.unpack drystal.colors['#fff']}

	it 'is case insensitive', ->
		assert.same {table.unpack drystal.colors.Blue }, {0, 0, 255}
		assert.same {table.unpack drystal.colors['#0000Ff']}, {0, 0, 255}

	it 'throws an error if the color is unknown', ->
		assert.error -> drystal.colors.flava

	it 'can be made darker', ->
		c = drystal.colors.gray
		c2 = c\darker!
		assert.is_true c2.r < c.r
		assert.is_true c2.g < c.g
		assert.is_true c2.b < c.b

	it 'can be made lighter', ->
		c = drystal.colors.gray
		c2 = c\lighter!
		assert.is_true c2.r > c.r
		assert.is_true c2.g > c.g
		assert.is_true c2.b > c.b

	-- TODO: conversions, operators

