RED = { 230, 30, 30 }
BLACK = { 20, 20, 20 }
DARK_GRAY = { 90, 90, 90 }
GRAY = { 150, 150, 150 }
WHITE = { 240, 240, 240 }

function draw_frame(x, y, w, h, border_color, inside_color, border_size)
	set_color(border_color)
	draw_rect(x, y, w, h)
	set_color(inside_color)
	draw_rect(x+border_size, y+border_size, w-border_size*2, h-border_size*2)
end

BAR = 0
CIRCLE = 1
function progress(x, y, w, h)
	local bar = {
		x=x, y=y, w=w, h=h,
		ratio = 0,
		border_size = 2,
		inside_color = GRAY,
		border_color = BLACK,
		progress_color = RED,
		text_color = WHITE,
		type=BAR,
		draw_ration=false,
	}
	bar.draw = function()
		local x, y, w, h = bar.x, bar.y, bar.w, bar.h
		local border_size = bar.border_size
		if bar.type == BAR then
			if bar.inside_color then
				set_fill(true)
				set_color(bar.inside_color)
				draw_rect(x, y, w, h)
			end
			if bar.border_color then
				set_fill(false)
				set_color(bar.border_color)
				draw_rect(x, y, w, h)
			end
			if bar.progress_color then
				set_fill(true)
				set_color(bar.progress_color)
				draw_rect(x + border_size, y + border_size, (w - border_size*2) * bar.ratio, h - border_size*2)
			end
			if bar.draw_ratio then
				set_color(bar.text_color)
				draw_centered_text(tostring(math.floor(bar.ratio * 100) .. '%'), x+w/2, y+h/2)
			end
		elseif bar.type == CIRCLE then
			assert(h ~= 0, 'elipsis aren\'t supported yet')
			a = bar.ratio * 360 - 90
			if bar.inside_color then
				set_fill(true)
				set_color(bar.inside_color)
				draw_circle(x, y, w)
			end
			if bar.progress_color then
				set_fill(true)
				set_color(bar.progress_color)
				draw_arc(x, y, w, -90, a)
			end
			if bar.border_color then
				set_fill(false)
				set_color(bar.border_color)
				draw_circle(x, y, w)
			end
		end
	end
	return bar
end

function draw_centered_text(text, x, y, add)
	local w, h = text_size(text)
	draw_text(text, x - w/2, y - h/2)
	if add ~= nil then
		draw_text(add, x + w/2, y - h/2)
	end
end

local bins = {}
function binomial(n, k)
	if k < 0 or k > n then
		return 0
	end
	if k > n - k then
		k = n - k
	end
	if bins[n] ~= nil and bins[n][k] ~= nil then
		return bins[n][k]
	end
	c = 1
	local floor = math.floor
	for i = 1, k do
		c = c * (n - (k - i))
		c = floor(c / i)
	end
	bins[n] = bins[n] or {}
	bins[n][k] = c
	return c
end

local blends = {}
function bezier_blend(n, t, i)
	if blends[n] ~= nil and blends[n][t] ~= nil and blends[n][t][i] ~= nil then
		return blends[n][t][i]
	end
	local blend = binomial(n, i) * (1 - t)^(n - i) * t^i
	blends[n] = blends[n] or {}
	blends[n][t] = blends[n][t] or {}
	blends[n][t][i] = blend
	return blend
end

local bezier_mu = 0.01

function set_bezier_mu(mu)
	if mu < 0.001 then
		mu = 0.001
	end
	bezier_mu = mu
end

function get_bezier(points)
	local n = #points - 1
	local bezier = function(t)
		x, y = 0, 0
		for ii, p in ipairs(points) do
			i = ii - 1
			local blend = bezier_blend(n, t, i)
			x = x + blend * p[1]
			y = y + blend * p[2]
		end
		return x, y
	end
	local list = {}
	for t = 0, 1, bezier_mu do
		if n == 2 then
			x = (1-t)^2*points[1][1] + 2*t*(1-t)*points[2][1] + t^2*points[3][1]
			y = (1-t)^2*points[1][2] + 2*t*(1-t)*points[2][2] + t^2*points[3][2]
		else
			x, y = bezier(t)
		end
		list[#list+1] = {x, y}
	end
	list[#list+1] = points[#points]
	return list
end

function get_bezier_fast(points)
	return get_bezier(points)
end

function draw_bezier(points)
	draw_segments(get_bezier(points))
end

function draw_segments(points, last_index)
	local last = nil
	if last_index == nil then
		last_index = #points
	end
	for i = 1, last_index do
		local p = points[i]
		if i ~= 1 then
			draw_line(last[1], last[2], p[1], p[2])
		end
		last = p
	end
end

