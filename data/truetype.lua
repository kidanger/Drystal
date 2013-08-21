-- add the following functions
-- - draw_align(text, x, y, alignment)
--		alignement = center | right | anythingelse = left
--		use drystal'.lua offset if loaded
-- - use_color(use)
--		replace draw with draw_color and use draw_color in draw_align
--		replace sizeof too
--		no color draw is still usable via truetype.rawdraw

-- remove Lua loading to make sure shared lib is loaded
local _path = package.path
package.path = ''

local truetype = require 'truetype'
package.path = _path

truetype.rawdraw = truetype.draw
truetype.rawsizeof = truetype.sizeof

local color = false

function truetype.use_color(col)
	color = col
	if color then
		truetype.draw = truetype.draw_color
		truetype.sizeof = truetype.sizeof_color
	else
		truetype.draw = truetype.rawdraw
		truetype.sizeof = truetype.rawsizeof
	end
end

function truetype.draw_align(text, x, y, alignement)
	if alignement == 'center' then
		local w = truetype.sizeof(text)
		x = x - w / 2
	elseif alignement == 'right' then
		local w = truetype.sizeof(text)
		x = x - w
	end
	if get_offset then
		local ox, oy = get_offset()
		x = x + ox
		y = y + oy
	end
	truetype.draw(text, x, y)
end

return truetype

