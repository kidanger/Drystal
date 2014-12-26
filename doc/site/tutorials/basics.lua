local drystal = require 'drystal'

local W, H = 400, 400

-- load resources
local font = assert(drystal.load_font('arial.ttf', 40))
local smallfont = assert(drystal.load_font('arial.ttf', 24))
local spritesheet = drystal.load_surface('spritesheet.png')
spritesheet:set_filter(drystal.BILINEAR)
local sprites = {
	{ x=0, y=0, w=128, h=128 }, -- O
	{ x=128, y=0, w=128, h=128 }, -- X
}

-- game info
local ended = false
local current_player = 'x'
local board = {
	{'', '', ''},
	{'', '', ''},
	{'', '', ''},
}
local highlighted_tile = {i=-1, j=-1}

function drystal.init()
	drystal.resize(W, H)
end

local function draw_board()
	drystal.set_color(drystal.colors.white)
	-- horizontal lines
	drystal.draw_line(0, H * 1 / 3, W, H * 1 / 3)
	drystal.draw_line(0, H * 2 / 3, W, H * 2 / 3)
	-- vertical lines
	drystal.draw_line(W * 1 / 3, 0, W * 1 / 3, H)
	drystal.draw_line(W * 2 / 3, 0, W * 2 / 3, H)
end

local function draw_marks()
	spritesheet:draw_from()
	for i = 1, 3 do
		for j = 1, 3 do
			local mark = board[i][j]
			local x = W * (j - 1) / 3
			local y = H * (i - 1) / 3
			if mark == 'x' then
				drystal.set_alpha(255)
				drystal.draw_sprite_resized(sprites[2], x, y, W / 3, H / 3)
			elseif mark == 'o' then
				drystal.set_alpha(255)
				drystal.draw_sprite_resized(sprites[1], x, y, W / 3, H / 3)
			elseif i == highlighted_tile.i and j == highlighted_tile.j then
				drystal.set_alpha(100)
				local sprite = current_player == 'x' and 2 or 1
				drystal.draw_sprite_resized(sprites[sprite], x, y, W / 3, H / 3)
			end
		end
	end
end

local function draw_gameover()
	-- fade the screen
	drystal.set_alpha(150)
	drystal.set_color(drystal.colors.black)
	drystal.draw_rect(0, 0, W, H)

	local text
	if current_player == 'x' then
		text = "Player 1 won the game!"
	elseif current_player == 'o' then
		text = "Player 2 won the game!"
	else
		text = "It's a tie!"
	end
	local _, htext = font:sizeof(text)
	drystal.set_alpha(255)
	drystal.set_color(drystal.colors.white)
	font:draw(text, W / 2, H / 2 - htext / 2, drystal.ALIGN_CENTER)

	smallfont:draw('Click to restart', W / 2, H * .7, drystal.ALIGN_CENTER)
end

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(drystal.colors.black)
	drystal.draw_background()

	draw_board()
	draw_marks()

	-- draw the winner
	if ended then
		draw_gameover()
	end
end

local function check_winner()
	-- horizontal lines
	for i = 1, 3 do
		if board[i][1] ~= '' and board[i][1] == board[i][2] and board[i][2] == board[i][3] then
			return true
		end
	end
	-- vertical lines
	for j = 1, 3 do
		if board[1][j] ~= '' and board[1][j] == board[2][j] and board[2][j] == board[3][j] then
			return true
		end
	end
	-- diagonal lines
	if board[1][1] ~= '' and board[1][1] == board[2][2] and board[2][2] == board[3][3] then
		return true
	end
	if board[3][1] ~= '' and board[3][1] == board[2][2] and board[2][2] == board[1][3] then
		return true
	end
	return false
end

local function check_board_full()
	for i = 1, 3 do
		for j = 1, 3 do
			if board[i][j] == '' then
				return false
			end
		end
	end
	return true
end

local function play()
	local i, j = highlighted_tile.i, highlighted_tile.j

	-- is there already a nought or a cross on this tile ?
	if board[i][j] ~= '' then
		return
	end

	-- set the tile
	board[i][j] = current_player

	if check_winner() then
		ended = true
	elseif check_board_full() then
		ended = true
		current_player = ''
	else
		-- switch player
		if current_player == 'x' then
			current_player = 'o'
		else
			current_player = 'x'
		end
	end
end

local function restart()
	ended = false
	current_player = 'x'
	board = {
		{'', '', ''},
		{'', '', ''},
		{'', '', ''},
	}
end

function drystal.mouse_motion(x, y)
	highlighted_tile = {
		i = math.ceil(y / W * 3),
		j = math.ceil(x / H * 3),
	}
end

function drystal.mouse_press(x, y, button)
	if button == drystal.BUTTON_LEFT then
		if ended then
			restart()
		else
			play()
		end
	end
end
