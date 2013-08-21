require 'drystal'
require 'draw'

local json = require 'dkjson'
spritesheet = json.decode(io.open('image.json'):read('*all'))

math.randomseed(os.time())

width = width or 500
height = height or 400

HUMAN = 0
BLOCK = 1

Entity = Entity or {}
Entity.__index = Entity

function Entity.new(type, static)
	if static == nil then static = true end
	local e = setmetatable({}, Entity)
	e.x, e.y = 0, 0
	e.dx, e.dy = 0, 0
	e.speedx, e.speedy = 0.25, 0.09
	e.dead = 0
	e.sprite = spritesheet.frames[type == HUMAN and 'character.png' or 'block.png'].frame
	e.type = type
	e.static = static
	e.jmp, e.jmp_timer = false, 0
	e.collides = { up=false, down=false, right=false, left=false }
	e.left, e.right = false, false
	e.chunk = nil
	return e
end

function Entity:kill()
	self.dead = true
	if self.chunk then
		self.chunk:remove(self)
	end
end

function Entity:goright()
	self.right = true
	local airfactor = 0.8
	if self.collides.down then
		airfactor = 1
	end
	self.dx = airfactor
end
function Entity:goleft()
	self.left = true
	local airfactor = 0.8
	if self.collides.down then
		airfactor = 1
	end
	self.dx = -1 * airfactor
end
function Entity:stopright()
	self.right = false
	local airfactor = 0.8
	if self.collides.down then
		airfactor = 1
	end
	if self.left then
		self.dx = -1 * airfactor
	else
	end
end
function Entity:stopleft()
	self.left = false
	local airfactor = 0.8
	if self.collides.down then
		airfactor = 1
	end
	if self.right then
		self.dx = 1 * airfactor
	else
	end
end

function Entity:gojmp()
	if self.collides.down then
		self.dy = self.dy - 3.9
		self.jmp = true
		self.jmp_timer = 9
	end
end
function Entity:stopjmp()
	self.jmp_timer = 0
end

function Entity:update(dt)
	self.jmp = self.jmp and self.jmp_timer > 0
	if self.jmp then
		self.jmp_timer = self.jmp_timer - 1
		self.dy = self.dy - 0.13
	end
	-- friction
	if not self.right and not self.left then
		if self.collides.down then
			self.dx = self.dx * 0.30
		else
			self.dx = self.dx * 0.75
		end
	end
	self.x = self.x + self.dx * self.speedx * dt
	self.y = self.y + self.dy * self.speedy * dt
	if not self.static then
		-- up collision
		local collide = false
		local collide_with = nil
		do
			local e = chunkmanager:get_near(self.x, self.y+16, self)
			if e then
				if math.abs(self.x - e.x) <= 24 and self.y + 16 < e.y and self.y + 32 > e.y then
					collide = true
					collide_with = e
				end
			end
		end
		if collide and (not self.jmp or self.dy > 0) then
			self.dy = 0
			self.y = collide_with.y - 32 + 0.1 -- on s'enfonce dans le sol
			self.collides.down = true
		else
			self.dy = self.dy + 0.25
			self.collides.down = false
		end
		-- down collision
		local collide = false
		local collide_with = nil
		local collide_with = nil
		do
			local e = chunkmanager:get_near(self.x, self.y-16, self)
			if e then
				if math.abs(self.x - e.x) <= 24 and self.y - 16 > e.y and self.y - 32 < e.y then
					collide = true
					collide_with = e
				end
			end
		end
		if collide then
			if self.dy < 0 then
				if not collide_with.static then
					collide_with.dy = self.dy * 0.7
				end
				self.dy = 0
				self.y = collide_with.y + 32 - 0.1
				self.collides.up = true
				self.jmp_timer = 0
			end
		else
			self.collides.up = false
		end

		-- right collision
		local collide = false
		local collide_with = nil
		do
			local e = chunkmanager:get_near(self.x-8, self.y, self)
			if e then
				if math.abs(self.y - e.y) <= 31 and self.x - 16 > e.x and self.x - 32 < e.x then
					collide = true
					collide_with = e
				end
			end
		end
		if collide then
			if self.dx <= 0 then
				self.x = collide_with.x + 32
				self.collides.left = true
				if not collide_with.static then
					collide_with.dx = self.dx
				end
			end
		else
			self.collides.left = false
		end
		-- left collision
		local collide = false
		local collide_with = nil
		do
			local e = chunkmanager:get_near(self.x+8, self.y, self)
			if e then
				if math.abs(self.y - e.y) <= 24 and self.x + 16 < e.x and self.x + 32 > e.x then
					collide = true
					collide_with = e
				end
			end
		end
		if collide then
			if self.dx >= 0 then
				self.x = collide_with.x - 32
				self.collides.right = true
				if not collide_with.static then
					collide_with.dx = self.dx
				end
			end
		else
			self.collides.right = false
		end
	end
	self:check_chunk()
end

function Entity:check_chunk()
	local old = self.chunk
	local new = chunkmanager:chunk_at(self.x, self.y, true)
	if old ~= new then
		old:remove(self)
		new:add(self)
	end
end

function make_human()
	local e = Entity.new(HUMAN, false)
	return e
end
function make_block(static)
	local e = Entity.new(BLOCK, static)
	return e
end

local CS = 32 * 5
Chunk = Chunk or {}
Chunk.__index = Chunk

chunkmanager = chunkmanager or {
	chunks={},
}
function chunkmanager:add(e)
	local x, y = e.x, e.y
	local c = self:chunk_at(x, y, true)
	c:add(e)
end

function chunkmanager:chunk_at(x, y, create)
	x = math.floor(x / CS) * CS - 16
	y = math.floor(y / CS) * CS - 16
	self.chunks[x] = self.chunks[x] or {}
	if create == true then
		self.chunks[x][y] = self.chunks[x][y] or Chunk.new(x, y)
	end
	return self.chunks[x][y]
end

function chunkmanager:get_near(x, y, _not)
	best = nil
	dist = 0
	local function test(xx, yy)
		local c = self:chunk_at(xx, yy)
		if not c then
			return nil
		end
		local e, d = c:get_near(x, y, _not)
		if e == nil then
			return
		end
		local dx, dy = x-e.x, y-e.y
		local dd = dx*dx + dy*dy
		if d < dist or not best then
			best = e
			dist = d
		end
	end
	local x2, y2 = x, y
	if x % CS < CS / 2 then
		x2 = x - CS
	else
		x2 = x + CS
	end
	if y % CS < CS / 2 then
		y2 = y - CS
	else
		y2 = y + CS
	end
	test(x, y)
	test(x, y2)
	test(x2, y)
	test(x2, y2)
	return best
end

function chunkmanager:render()
	local px, py = player.x, player.y
	local i, j = 0, 0
	local stx = math.floor((px - width*0.5) / CS) * CS - 16
	local enx = math.ceil((px + width*0.5) / CS) * CS
	local sty = math.floor((py - height*0.5) / CS) * CS - 16
	local eny = math.ceil((py + height*0.5) / CS - 0.4) * CS
	for x = stx, enx, CS do
		for y = sty, eny, CS do
			local c = self.chunks[x] and self.chunks[x][y]
			if c then c:render(); j = j + 1 end
		end
	end
end

function chunkmanager:update(dt)
	local px, py = player.x, player.y
	for _, cc in pairs(self.chunks) do
		for _, c in pairs(cc) do
			if (c.x > px - width*0.5 - CS and c.x < px + width*0.5 + CS
				and c.y > py - height*0.5 - CS and c.y < py + height*0.5 + CS) then
				c:update(dt)
			end
		end
	end
end

function chunkmanager:reset()
	self:clean()
	self.chunks = {}
end
function chunkmanager:clean()
	for _, cc in pairs(self.chunks) do
		for _, c in pairs(cc) do
			if c.map then
				free_surface(c.map)
				c.map = nil
			end
		end
	end
end

function Chunk.new(x, y)
	local c = setmetatable({}, Chunk)
	c.x, c.y = x, y
	c.w, c.h = CS, CS
	c.blocks = {}
	c.entities = {}
	c.map = nil
	return c
end

function Chunk:get_near(x, y, _not)
	local best = nil
	local dist = 0
	function check(e)
		local dx, dy = x-e.x, y-e.y
		local dd = dx*dx + dy*dy
		if (dd < dist or not best) and e ~= _not then
			best = e
			dist = dd
		end
	end
	for _, b in pairs(self.blocks) do
		check(b)
	end
	for _, e in pairs(self.entities) do
		check(e)
	end
	return best, dist
end

function Chunk:add(e)
	if e.static then
		self.blocks[#self.blocks + 1] = e
	else
		self.entities[#self.entities + 1] = e
	end
	e.chunk = self
end

function Chunk:remove(e)
	local list = nil
	if e.static then
		list = self.blocks
	else
		list = self.entities
	end
	for k, ee in pairs(list) do
		if e == ee then
			table.remove(list, k)
			break
		end
	end
end

function Chunk:render()
	local x, y = self.x, self.y
	set_color(255, 255, 255)
	set_alpha(255)
	for _, e in pairs(self.blocks) do
		draw_sprite(e.sprite, e.x-16, e.y-16)
	end

	for _, e in pairs(self.entities) do
		draw_sprite(e.sprite, e.x-16, e.y-16)
	end

	set_color(200, 50, 50)
	set_alpha(50)
	draw_square(x, y, CS, CS)
	set_alpha(255)

	for _, e in pairs(self.entities) do
		set_color(200, 50, 50)
		local r = 2
		if e.collides.up then
			draw_circle(e.x, e.y - 16, r)
		end
		if e.collides.down then
			draw_circle(e.x, e.y + 16, r)
		end
		if e.collides.left then
			draw_circle(e.x - 16, e.y, r)
		end
		if e.collides.right then
			draw_circle(e.x + 16, e.y, r)
		end
	end
end

function Chunk:update(dt)
	for _, e in pairs(self.entities) do
		e:update(dt)
	end
end

function load_map(filename)
	local x, y = 0, 0
	for line in io.lines(filename) do
		for c in line:gmatch('.') do
			local e = nil
			if c == 'X' then
				goto done
			elseif c == '#' then
				e = make_block(true)
			elseif c == '0' then
				e = make_block(false)
			elseif c == 'B' then
				e = make_human()
			elseif c == 'S' then
				player.x = x
				player.y = y
			end
			if e ~= nil then
				e.x = x
				e.y = y
				chunkmanager:add(e)
			end
			x = x + 32
		end
		y = y + 32
		x = 0
	end
	::done::
	loaded = true
end

function reload()
	if not loaded then
		local x, y = 0, 0
		local pp = false
		if player ~= nil then
			x = player.x
			y = player.y
			pp = true
		end
		chunkmanager:reset()
		player = make_human()
		load_map('map')
		chunkmanager:add(player)

		if pp then
			player.x = x
			player.y = y
		end
	end
end


function init()
	set_resizable(true)
	resize(width, height)

	reload()
	if mscreen then free_surface(mscreen) end
	mscreen = new_surface(surface_size(screen))
	atlas = load_surface('image.png')
	draw_from(atlas)
end

function resize_event(w, h)
	width = w
	height = h
	resize(w, h)
	if mscreen then free_surface(mscreen) end
	mscreen = new_surface(surface_size(screen))
	if atlas then free_surface(atlas); end
	atlas = load_surface('image.png')
	draw_from(atlas)
end

local tick = tick or 0
fps = fps or 0

function update(dt)
	tick = tick + dt
	if dt >= 1000 then
		fps = 0
	elseif dt > 0 then
		fps = fps * 0.95 + (1000 / dt) * 0.05
	end
	chunkmanager:update(dt)
end

function draw()
	draw_on(mscreen)
	set_color(0,0,0)
	draw_background()
	set_color(150,150,150)

	local ox = -player.x - 16 + width / 2
	local oy = -player.y - 16 + height / 2
	push_offset(ox, oy)

	chunkmanager:render()

	pop_offset()
	draw_on(screen)
	set_color(0,0,0)
	draw_background()
	set_color(255,255,255)
	local sx, sy = surface_size(mscreen)
	draw_surface(mscreen, math.random(-1,1)*player.dy/5, 0)
	flip()
end

function mouse_press(x, y, button)
	local ox = player.x + 16 - width*0.5
	local oy = player.y + 16 - height*0.5
	if button == 1 or button == 3 then
		local e = make_block(button == 1)
		e.x = ox + math.floor(x/8)*8 + 4
		e.y = oy + math.floor(y/8)*8 + 4
		chunkmanager:add(e)
	else
		local e = chunkmanager:get_near(ox + x, oy + y)
		if e ~= nil and e ~= player then
			e:kill()
		end
	end
end

function key_press(key)
	if key == 'q' then
		player:goleft()
	end
	if key == 'd' then
		player:goright()
	end
	if key == 'space' then
		player:gojmp()
	end
	if key == 'k' then
		player:kill()
	end
	if key == 'r' then
		loaded = false
		reload()
	end
	if key == 't' then
		loaded = false
		player = nil
		reload()
	end
	if key == 'a' then
		engine_stop()
	end
end

function key_release(key)
	if key == 'space' then
		player:stopjmp()
	elseif key == 'd' then
		player:stopright()
	elseif key == 'q' then
		player:stopleft()
	end
end
