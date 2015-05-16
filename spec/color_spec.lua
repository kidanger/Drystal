local drystal = require 'drystal'

describe('color', function()

	it('has W3C colors', function()
		assert.same({255, 0, 0}, {table.unpack(drystal.colors.red)})
		assert.same({0, 0, 0}, {table.unpack(drystal.colors.black)})
	end)

	it('supports hexadecimal formatting', function()
		assert.same({0, 0, 0}, {table.unpack(drystal.colors['#000000'])})
		assert.same({255, 255, 255}, {table.unpack(drystal.colors['#ffffff'])})
		assert.same({15, 15, 15}, {table.unpack(drystal.colors['#0f0f0f'])})
		assert.same({16, 16, 16}, {table.unpack(drystal.colors['#101010'])})
		assert.same({255, 255, 255}, {table.unpack(drystal.colors['#fff'])})
	end)

	it('is case insensitive', function()
		assert.same({table.unpack(drystal.colors.Blue)}, {0, 0, 255})
		assert.same({table.unpack(drystal.colors['#0000Ff'])}, {0, 0, 255})
	end)

	it('throws an error if the color is unknown', function()
		assert.error(function()
			local _ = drystal.colors['flava']
		end)
	end)

	it('can be made darker', function()
		local c = drystal.colors.gray
		local c2 = c:darker()
		assert.is_true(c2.r < c.r)
		assert.is_true(c2.g < c.g)
		assert.is_true(c2.b < c.b)
	end)

	it('can be made lighter', function()
		local c = drystal.colors.gray
		local c2 = c:lighter()
		assert.is_true(c2.r > c.r)
		assert.is_true(c2.g > c.g)
		assert.is_true(c2.b > c.b)
	end)

	it('can be made lighter', function()
		local c = drystal.colors.gray
		local c2 = c:lighter()
		assert.is_true(c2.r > c.r)
		assert.is_true(c2.g > c.g)
		assert.is_true(c2.b > c.b)
	end)

	-- TODO: conversions, operators

end)

