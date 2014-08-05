local drystal = require 'drystal'

local postfxs = {}
local backsurface
local builtin_postfx = {}

function drystal.create_postfx(name, code, uniforms)
	if not drystal.screen then
		return {} -- server mode probably
	end
	uniforms = uniforms or {}
	local uniforms_code = ''
	for i, name in ipairs(uniforms) do
		uniforms_code = uniforms_code .. [[
			uniform float ]] .. name .. [[;
		]]
	end
	code = [[
		varying vec2 fTexCoord;
		uniform sampler2D tex;
		]] .. uniforms_code .. [[
		]] .. code .. [[
		void main()
		{
			gl_FragColor = vec4(effect(tex, fTexCoord), 1.0);
		}
	]]
	local shader, err = drystal.new_shader(nil, nil, code)
	if not shader then
		return nil, err
	end
	local fx = function(...)
		drystal.set_color(255, 255, 255)
		drystal.set_alpha(255)

		shader:use()
		for i, u in ipairs(uniforms) do
			local v = select(i, ...) or 0
			shader:feed(u, v)
		end
		local surface = backsurface:draw_on()
		local old = surface:draw_from()
		drystal.draw_image(0, 0, backsurface.w, backsurface.h, 0, 0)

		drystal.use_shader()
		backsurface:draw_from()
		surface:draw_on()
		drystal.draw_image(0, 0, backsurface.w, backsurface.h, 0, 0)

		if old then
			old:draw_from()
		end
	end
	postfxs[name] = fx
	return fx
end

local function add_postfx(name, code, uniforms)
	builtin_postfx[name] = {
		code = code,
		uniforms = uniforms
	}
end

add_postfx('gray', [[
	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec3 texval = texture2D(tex, coord).rgb;
		return mix(texval, vec3((texval.r + texval.g + texval.b) / 3.0), scale);
	}
]], {'scale',})

add_postfx('red', [[
	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec3 texval = texture2D(tex, coord).rgb;
		return vec3(red, 0., 0.) + texval;
	}
]], {'red',})

add_postfx('distortion', [[
	#define pi ]] .. math.pi .. [[

	vec3 effect(sampler2D tex, vec2 coord)
	{
		coord.x += sin(coord.y * 8.*pi + time * 2. * pi * .75) / 100.;
		return texture2D(tex, coord).rgb;
	}
]], {'time',})

add_postfx('blurDir', [[
	const float weight1 = 0.3989422804014327;
	const float weight2 = 0.24197072451914536;
	const float weight3 = 0.05399096651318985;
	const float weight4 = 0.004431848411938341;

	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec2 dir = vec2(dx, dy);
		vec3 acc = vec3(0., 0., 0.);

		acc += texture2D(tex, coord).rgb * weight1;

		acc += texture2D(tex, coord + dir).rgb * weight2;
		acc += texture2D(tex, coord - dir).rgb * weight2;

		acc += texture2D(tex, coord + dir*2.).rgb * weight3;
		acc += texture2D(tex, coord - dir*2.).rgb * weight3;

		acc += texture2D(tex, coord + dir*3.).rgb * weight4;
		acc += texture2D(tex, coord - dir*3.).rgb * weight4;

		acc /= weight1 + (weight2 + weight3 + weight4) * 2.;
		return acc;
	}
]], {'dx', 'dy',})

postfxs.blur = function(...)
	drystal.postfx('blurDir', 1 / drystal.current_draw_on.w, 0)
	drystal.postfx('blurDir', 0, 1 / drystal.current_draw_on.h)
end

add_postfx('vignette', [[
	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec2 m = vec2(0.5, 0.5);
		float d = distance(m, coord);
		vec3 texval = texture2D(tex, coord).rgb;
		return texval * smoothstep(outer, inner, d);
	}
]], {'outer', 'inner',})

add_postfx('dither', [[
	/*
		Port of shader by Ceaphyrel, found at
		http://www.assembla.com/code/MUL2010_OpenGLScenePostprocessing/subversion/nodes/MUL%20FBO/Shaders/dithering.frag?rev=83
		toneburst 2011
	*/
	float find_closest(int x, int y, float c0)
	{
		mat4 dither = mat4(
			1.0,  33.0,  9.0, 41.0,
			49.0, 17.0, 57.0, 25.0,
			13.0, 45.0,  5.0, 37.0,
			61.0, 29.0, 53.0, 21.0 );

		float limit = 0.0;
		for (int xx = 0; xx < 8; xx++)
		{
			if (xx == x)
			{
				for (int yy = 0; yy < 8; yy++)
				{
					if (yy == y)
					{
						if(x < 4) {
							if(y >= 4) {
								limit = (dither[xx][yy-4]+3.0)/65.0;
							} else {
								limit = (dither[xx][yy])/65.0;
							}
						}
						if(x >= 4) {
							if(y >= 4)
								limit = (dither[xx-4][yy-4]+1.0)/65.0;
							else
								limit = (dither[xx-4][yy]+2.0)/65.0;
						}
						break;
					}
				}
				break;
			}
		}
		if(c0 < limit)
			return 0.0;

		return 1.0;
	}

	vec3 effect(sampler2D tex, vec2 coord)
	{
		vec4 texval = texture2D(tex, coord.xy);
		float grayscale = dot(texval, vec4(0.299, 0.587, 0.114, 0));
		vec2 xy = coord * scale;

		int x = int(mod(xy.x, 8.0));
		int y = int(mod(xy.y, 8.0));
		float final = find_closest(x, y, grayscale);
		return final * texval.rgb;
	}
]], {'scale',})

add_postfx('pixelate_', [[
	vec3 effect(sampler2D tex, vec2 coord) {
		vec2 size = vec2(sizex, sizey) * vec2(dx, dy);
		vec2 c = size * floor(coord/size);
		return texture2D(tex, c).rgb;
	}
]], {'sizex', 'sizey', 'dx', 'dy'})

postfxs.pixelate = function(sizex, sizey)
	drystal.postfx('pixelate_', sizex, sizey,
				1 / drystal.current_draw_on.w, 1 / drystal.current_draw_on.h)
end

function drystal.postfx(name, ...)
	if not postfxs[name] then
		if builtin_postfx[name] then
			local postfx = builtin_postfx[name]
			assert(drystal.create_postfx(name, postfx.code, postfx.uniforms))
		else
			error('Post FX ' .. name .. ' not found.')
		end
	end
	local surface = drystal.current_draw_on
	if not backsurface or backsurface.w ~= surface.w or backsurface.h ~= surface.h then
		backsurface = drystal.new_surface(surface.w, surface.h, true)
	end
	postfxs[name](...)
end
