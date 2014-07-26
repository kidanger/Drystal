-- adds the following functions
-- - draw_align(text, x, y, alignment)
--		alignement = center | right | anythingelse = left
--		use drystal'.lua offset if loaded
-- - use_color(use)
--		replace draw with draw_plain and use draw_plain in draw_align
--		replace sizeof too
local drystal = require 'drystal'

local Font = drystal.Font

function Font:draw_align(text, x, y, alignement)
	local w, h = self:sizeof(str)
	if alignement == 'center' then
		x = x - w / 2
	elseif alignement == 'right' then
		x = x - w
	end
	self:draw(str, x, y)
end

function Font:draw_plain_align(text, x, y, alignement)
	local w, h = self:sizeof_plain(str)
	if alignement == 'center' then
		x = x - w / 2
	elseif alignement == 'right' then
		x = x - w
	end
	self:draw_plain(str, x, y)
end

