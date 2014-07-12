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
	oldx = x
	for str in text:gmatch("[^\n]*") do
		local w, h = truetype.sizeof(str)
		if alignement == 'center' then
			x = x - w / 2
		elseif alignement == 'right' then
			x = x - w
		end
		truetype.draw(str, x, y)
		y = y + h
		x = oldx
	end
end

return truetype

