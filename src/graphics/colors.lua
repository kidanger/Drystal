local drystal = require 'drystal'

local Color = {}
drystal.Color = Color
Color.__index = Color
Color.__add = function(a, b)
	return a:add(b)
end
Color.__sub = function(a, b)
	return a:sub(b)
end
Color.__mul = function(a, b)
	return a:mul(b)
end

local function rgb_to_hsl(r, g, b)
	-- See http://axonflux.com/handy-rgb-to-hsl-and-rgb-to-hsv-color-model-c
	r, g, b = r / 255, g / 255, b / 255

	local max, min = math.max(r, g, b), math.min(r, g, b)
	local h, s, l = 0, 0, (max + min) / 2

	if max == min then
		h, s = 0, 0 -- achromatic
	else
		local d = max - min
		if l > 0.5 then s = d / (2 - max - min) else s = d / (max + min) end
		if max == r then
			h = (g - b) / d
			if g < b then h = h + 6 end
		elseif max == g then h = (b - r) / d + 2
		elseif max == b then h = (r - g) / d + 4
		end
		h = h / 6
	end

	return h * 360, s, l
end

local function hue_to_rgb(p, q, t)
	if t < 0   then t = t + 1 end
	if t > 1   then t = t - 1 end
	if t < 1/6 then return p + (q - p) * 6 * t end
	if t < 1/2 then return q end
	if t < 2/3 then return p + (q - p) * (2/3 - t) * 6 end
	return p
end

local function hsl_to_rgb(h, s, l)
	-- See http://axonflux.com/handy-rgb-to-hsl-and-rgb-to-hsv-color-model-c
	h = h / 360
	local r, g, b

	if s == 0 then
		r, g, b = l, l, l -- achromatic
	else
		local q
		if l < 0.5 then q = l * (1 + s) else q = l + s - l * s end
		local p = 2 * l - q

		r = hue_to_rgb(p, q, h + 1/3)
		g = hue_to_rgb(p, q, h)
		b = hue_to_rgb(p, q, h - 1/3)
	end

	return r * 255, g * 255, b * 255
end

local function rgb_to_cmyk(r, g, b)
	local k = math.min(255 - r, math.min(255 - g, 255 - b));
	local c = 100 * (255 - r - k) / (255 - k);
	local m = 100 * (255 - g - k) / (255 - k);
	local y = 100 * (255 - b - k) / (255 - k);
	return c, m, y, k
end

local function cmyk_to_rgb(c, m, y, k)
	local r = math.abs((c * (255 - k)) / 100 + k - 255);
	local g = math.abs((m * (255 - k)) / 100 + k - 255);
	local b = math.abs((y * (255 - k)) / 100 + k - 255);

	return r, g, b
end

function drystal.new_color(t, x, y, z, k)
	local r, g, b, h, s, l, c, m
	if type(t) == 'table' then
		x, y, z = table.unpack(t)
		t = 'rgb'
	elseif type(t) ~= 'string' then
		error('Color: unknown type')
	end

	if type(x) == 'table' then
		x, y, z, k = table.unpack(x)
	end

	if t == 'hsl' then
		h, s, l = x, y, z
		r, g, b = hsl_to_rgb(h, s, l)
		c, m, y, k = rgb_to_cmyk(r, g, b)
	elseif t == 'cmyk' then
		c, m, y, k = x, y, z, k
		r, g, b = cmyk_to_rgb(c, m, y, k)
		h, s, l = rgb_to_hsl(r, g, b)
	elseif t == 'rgb' then
		r, g, b = x, y, z
		h, s, l = rgb_to_hsl(r, g, b)
		c, m, y, k = rgb_to_cmyk(r, g, b)
	else
		error('Color: unknown type')
	end

	local color = {
		r,g,b,
		r=r,g=g,b=b,
		h=h,s=s,l=l,
		c=c,m=m,y=y,k=k,
	}
	return setmetatable(color, Color)
end

function Color:lighter()
	-- The max is not 1 otherwise, if we want the darkest blue, then when we lighten the blue it will be gray
	return drystal.new_color('hsl', self.h, self.s, math.min(0.99999, self.l + 0.05))
end

function Color:darker()
	-- The min is not 0 otherwise, if we want the lightest blue, then when we darken the blue it will be gray
	return drystal.new_color('hsl', self.h, self.s, math.max(0.00001, self.l - 0.05))
end

function Color:add(other)
	local c, m, y, k = self.c, self.m, self.y, self.k
	local c2, m2, y2, k2 = other.c, other.m, other.y, other.k

	return drystal.new_color {cmyk_to_rgb(c + c2, m + m2, y + y2, k + k2)}
end

function Color:sub(other)
	local c, m, y, k = self.c, self.m, self.y, self.k
	local c2, m2, y2, k2 = other.c, other.m, other.y, other.k

	return drystal.new_color {cmyk_to_rgb(c - c2, m - m2, y - y2, k - k2)}
end

function Color:mul(other)
	local r = self.r * other.r / 255
	local g = self.g * other.g / 255
	local b = self.b * other.b / 255
	return drystal.new_color('rgb', r, g, b)
end

function Color:cmyk()
	return self.c, self.m, self.y, self.k
end

function Color:rgb()
	return self.r, self.g, self.b
end

function Color:hsl()
	return self.h, self.s, self.l
end

local function decode_hex_number(str)
	local b, b2 = str:byte(1, 2)
	local n
	if b >= 97 then
		n = b - 97 + 10
	else
		n = b - 48
	end
	if b2 then
		if b2 >= 97 then
			n = n * 16 + b2 - 97 + 10
		else
			n = n * 16 + b2 - 48
		end
	else
		n = n * 16 + n
	end
	return n
end

local function hex_color(str)
	local lowstr = str:lower()
	if #str == 7 then
		r = decode_hex_number(lowstr:sub(2, 3))
		g = decode_hex_number(lowstr:sub(4, 5))
		b = decode_hex_number(lowstr:sub(6, 7))
	elseif #str == 4 then
		r = decode_hex_number(lowstr:sub(2, 2))
		g = decode_hex_number(lowstr:sub(3, 3))
		b = decode_hex_number(lowstr:sub(4, 4))
	else
		error("set_color: " .. str .. " doesn't follow the #rrggbb or #rgb format")
	end
	local color = drystal.new_color('rgb', r, g, b)
	drystal.colors[str] = color -- cache the value
	return color
end

-- See http://www.w3.org/TR/css3-color/#svg-color
drystal.colors = setmetatable ({
aliceblue            = drystal.new_color {240,248,255},
antiquewhite         = drystal.new_color {250,235,215},
aqua                 = drystal.new_color {0,255,255},
aquamarine           = drystal.new_color {127,255,212},
azure                = drystal.new_color {240,255,255},
beige                = drystal.new_color {245,245,220},
bisque               = drystal.new_color {255,228,196},
black                = drystal.new_color {0,0,0},
blanchedalmond       = drystal.new_color {255,235,205},
blue                 = drystal.new_color {0,0,255},
blueviolet           = drystal.new_color {138,43,226},
brown                = drystal.new_color {165,42,42},
burlywood            = drystal.new_color {222,184,135},
cadetblue            = drystal.new_color {95,158,160},
chartreuse           = drystal.new_color {127,255,0},
chocolate            = drystal.new_color {210,105,30},
coral                = drystal.new_color {255,127,80},
cornflowerblue       = drystal.new_color {100,149,237},
cornsilk             = drystal.new_color {255,248,220},
crimson              = drystal.new_color {220,20,60},
cyan                 = drystal.new_color {0,255,255},
darkblue             = drystal.new_color {0,0,139},
darkcyan             = drystal.new_color {0,139,139},
darkgoldenrod        = drystal.new_color {184,134,11},
darkgray             = drystal.new_color {169,169,169},
darkgreen            = drystal.new_color {0,100,0},
darkgrey             = drystal.new_color {169,169,169},
darkkhaki            = drystal.new_color {189,183,107},
darkmagenta          = drystal.new_color {139,0,139},
darkolivegreen       = drystal.new_color {85,107,47},
darkorange           = drystal.new_color {255,140,0},
darkorchid           = drystal.new_color {153,50,204},
darkred              = drystal.new_color {139,0,0},
darksalmon           = drystal.new_color {233,150,122},
darkseagreen         = drystal.new_color {143,188,143},
darkslateblue        = drystal.new_color {72,61,139},
darkslategray        = drystal.new_color {47,79,79},
darkslategrey        = drystal.new_color {47,79,79},
darkturquoise        = drystal.new_color {0,206,209},
darkviolet           = drystal.new_color {148,0,211},
deeppink             = drystal.new_color {255,20,147},
deepskyblue          = drystal.new_color {0,191,255},
dimgray              = drystal.new_color {105,105,105},
dimgrey              = drystal.new_color {105,105,105},
dodgerblue           = drystal.new_color {30,144,255},
firebrick            = drystal.new_color {178,34,34},
floralwhite          = drystal.new_color {255,250,240},
forestgreen          = drystal.new_color {34,139,34},
fuchsia              = drystal.new_color {255,0,255},
gainsboro            = drystal.new_color {220,220,220},
ghostwhite           = drystal.new_color {248,248,255},
gold                 = drystal.new_color {255,215,0},
goldenrod            = drystal.new_color {218,165,32},
gray                 = drystal.new_color {128,128,128},
green                = drystal.new_color {0,128,0},
greenyellow          = drystal.new_color {173,255,47},
grey                 = drystal.new_color {128,128,128},
honeydew             = drystal.new_color {240,255,240},
hotpink              = drystal.new_color {255,105,180},
indianred            = drystal.new_color {205,92,92},
indigo               = drystal.new_color {75,0,130},
ivory                = drystal.new_color {255,255,240},
khaki                = drystal.new_color {240,230,140},
lavender             = drystal.new_color {230,230,250},
lavenderblush        = drystal.new_color {255,240,245},
lawngreen            = drystal.new_color {124,252,0},
lemonchiffon         = drystal.new_color {255,250,205},
lightblue            = drystal.new_color {173,216,230},
lightcoral           = drystal.new_color {240,128,128},
lightcyan            = drystal.new_color {224,255,255},
lightgoldenrodyellow = drystal.new_color {250,250,210},
lightgray            = drystal.new_color {211,211,211},
lightgreen           = drystal.new_color {144,238,144},
lightgrey            = drystal.new_color {211,211,211},
lightpink            = drystal.new_color {255,182,193},
lightsalmon          = drystal.new_color {255,160,122},
lightseagreen        = drystal.new_color {32,178,170},
lightskyblue         = drystal.new_color {135,206,250},
lightslategray       = drystal.new_color {119,136,153},
lightslategrey       = drystal.new_color {119,136,153},
lightsteelblue       = drystal.new_color {176,196,222},
lightyellow          = drystal.new_color {255,255,224},
lime                 = drystal.new_color {0,255,0},
limegreen            = drystal.new_color {50,205,50},
linen                = drystal.new_color {250,240,230},
magenta              = drystal.new_color {255,0,255},
maroon               = drystal.new_color {128,0,0},
mediumaquamarine     = drystal.new_color {102,205,170},
mediumblue           = drystal.new_color {0,0,205},
mediumorchid         = drystal.new_color {186,85,211},
mediumpurple         = drystal.new_color {147,112,219},
mediumseagreen       = drystal.new_color {60,179,113},
mediumslateblue      = drystal.new_color {123,104,238},
mediumspringgreen    = drystal.new_color {0,250,154},
mediumturquoise      = drystal.new_color {72,209,204},
mediumvioletred      = drystal.new_color {199,21,133},
midnightblue         = drystal.new_color {25,25,112},
mintcream            = drystal.new_color {245,255,250},
mistyrose            = drystal.new_color {255,228,225},
moccasin             = drystal.new_color {255,228,181},
navajowhite          = drystal.new_color {255,222,173},
navy                 = drystal.new_color {0,0,128},
oldlace              = drystal.new_color {253,245,230},
olive                = drystal.new_color {128,128,0},
olivedrab            = drystal.new_color {107,142,35},
orange               = drystal.new_color {255,165,0},
orangered            = drystal.new_color {255,69,0},
orchid               = drystal.new_color {218,112,214},
palegoldenrod        = drystal.new_color {238,232,170},
palegreen            = drystal.new_color {152,251,152},
paleturquoise        = drystal.new_color {175,238,238},
palevioletred        = drystal.new_color {219,112,147},
papayawhip           = drystal.new_color {255,239,213},
peachpuff            = drystal.new_color {255,218,185},
peru                 = drystal.new_color {205,133,63},
pink                 = drystal.new_color {255,192,203},
plum                 = drystal.new_color {221,160,221},
powderblue           = drystal.new_color {176,224,230},
purple               = drystal.new_color {128,0,128},
red                  = drystal.new_color {255,0,0},
rosybrown            = drystal.new_color {188,143,143},
royalblue            = drystal.new_color {65,105,225},
saddlebrown          = drystal.new_color {139,69,19},
salmon               = drystal.new_color {250,128,114},
sandybrown           = drystal.new_color {244,164,96},
seagreen             = drystal.new_color {46,139,87},
seashell             = drystal.new_color {255,245,238},
sienna               = drystal.new_color {160,82,45},
silver               = drystal.new_color {192,192,192},
skyblue              = drystal.new_color {135,206,235},
slateblue            = drystal.new_color {106,90,205},
slategray            = drystal.new_color {112,128,144},
slategrey            = drystal.new_color {112,128,144},
snow                 = drystal.new_color {255,250,250},
springgreen          = drystal.new_color {0,255,127},
steelblue            = drystal.new_color {70,130,180},
tan                  = drystal.new_color {210,180,140},
teal                 = drystal.new_color {0,128,128},
thistle              = drystal.new_color {216,191,216},
tomato               = drystal.new_color {255,99,71},
turquoise            = drystal.new_color {64,224,208},
violet               = drystal.new_color {238,130,238},
wheat                = drystal.new_color {245,222,179},
white                = drystal.new_color {255,255,255},
whitesmoke           = drystal.new_color {245,245,245},
yellow               = drystal.new_color {255,255,0},
yellowgreen          = drystal.new_color {154,205,50},
}, {
	__index = function(self, key)
		if type(key) == 'string' then
			if key:sub(1, 1) == '#' then
				return hex_color(key)
			else
				local c = rawget(self, key:lower())
				if c then return c end
			end
		end
		error('color ' .. key .. ' does not exist', 3)
	end
})

