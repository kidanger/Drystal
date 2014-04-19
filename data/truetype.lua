-- add the following functions
-- - draw_align(text, x, y, alignment)
--		alignement = center | right | anythingelse = left
--		use drystal'.lua offset if loaded
-- - use_color(use)
--		replace draw with draw_plain and use draw_plain in draw_align
--		replace sizeof too

-- remove Lua loading to make sure shared lib is loaded
local _path = package.path
package.path = ''

local truetype = require 'truetype'
package.path = _path

truetype.rawdraw = truetype.draw
truetype.rawsizeof = truetype.sizeof

function truetype.use_color(col)
	if col then
		truetype.draw = truetype.rawdraw
		truetype.sizeof = truetype.rawsizeof
	else
		truetype.draw = truetype.draw_plain
		truetype.sizeof = truetype.sizeof_plain
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

