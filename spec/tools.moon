drystal = require 'drystal'

say = require 'say'
assert = require 'luassert'
util = require 'luassert.util'

fmt_color = (arg) ->
	if type(arg) == 'table' and arg.color
		if arg.name
			if arg[4]
				('{%q, %d}')\format(arg.name, arg[4])
			else
				('%q')\format(arg.name)
		elseif #arg == 3
			('{%d, %d, %d}')\format table.unpack arg
		elseif #arg == 4
			('{%d, %d, %d, %d}')\format table.unpack arg

is_color = (state, args) ->
	surface, x, y, color, alpha = table.unpack args
	alpha = alpha or 255

	assert(#args >= 4, say("assertion.internal.argtolittle", { "is_color", 4, #args }))
	assert(type(surface) == 'userdata' and surface.__type == 'surface', say("assertion.internal.badargtype", { "is_color", "surface", surface or 'nil'}))

	c = color
	if type(color) ~= 'table'
		c = { table.unpack drystal.colors[color] }
		c.name = color
	else
		c = { table.unpack c }
	c[4] = alpha
	c.color = true

	old_on = drystal.current_draw_on == surface
	other = surface == drystal.screen and drystal.new_surface(x, y) or drystal.screen
	other\draw_on! if old_on
	r, g, b, a = surface\get_pixel x, y
	surface\draw_on! if old_on

	args[1] = c
	args[2] = {r, g, b, a, color: true}

	r == c[1] and g == c[2] and b == c[3] and a == alpha

say\set_namespace "en"
say\set "assertion.color.positive", "Expected surface to be %s, was %s"

assert\register "assertion", "color", is_color, "assertion.color.positive"
assert\add_formatter fmt_color

