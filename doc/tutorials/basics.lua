local drystal = require 'drystal'

local W, H = 400, 400
drystal.resize(W, H)

-- load resources
local font = assert(drystal.load_font('arial.ttf', 40))
local smallfont = assert(drystal.load_font('arial.ttf', 24))
local spritesheet = drystal.load_surface('spritesheet.png')
spritesheet:set_filter(drystal.NEAREST)
local sprites = {
	{ x=0, y=0, w=32, h=32 }, -- X
	{ x=32, y=0, w=32, h=32 }, -- O
}

-- game info
local ended = false
local current_player = 1
local board = {
	{1, 0, 0},
	{0, 0, 0},
	{0, 2, 0},
}
local highlighted_tile = {-1, -1}

function drystal.draw()
	drystal.set_alpha(255)
	drystal.set_color(0, 10, 0)
	drystal.draw_background()
	
	-- draw the board
	drystal.set_color(255, 255, 255)
	-- horizontal lines
	drystal.draw_line(W * .05, H * 1 / 3, W * .95, H * 1 / 3)
	drystal.draw_line(W * .05, H * 2 / 3, W * .95, H * 2 / 3)
	-- vertical lines
	drystal.draw_line(W * 1 / 3, H * .05, W * 1 / 3, H * .95)
	drystal.draw_line(W * 2 / 3, H * .05, W * 2 / 3, H * .95)
	-- marks
	spritesheet:draw_from()
	for i = 1, 3 do
		for j = 1, 3 do
			local mark = board[i][j]
			if mark ~= 0 then
				drystal.set_alpha(255)
				local x = W * (i - 1) / 3
				local y = H * (j - 1) / 3
				drystal.draw_sprite_resized(sprites[mark], x, y, W / 3, H / 3)
			elseif highlighted_tile and i == highlighted_tile.i and j == highlighted_tile.j then
				drystal.set_alpha(100)
				local x = W * (i - 1) / 3
				local y = H * (j - 1) / 3
				drystal.draw_sprite_resized(sprites[current_player], x, y, W / 3, H / 3)
			end
		end
	end

	-- draw the winner
	if ended then
		-- fade the screen
		drystal.set_alpha(150)
		drystal.set_color(0, 0, 0)
		drystal.draw_rect(0, 0, W, H)

		local text
		if current_player == 1 then
			text = "Player 1 won the game!"
		elseif current_player == 2 then
			text = "Player 2 won the game!"
		else
			text = "It's a tie!"
		end
		local _, htext = font:sizeof(text)
		drystal.set_alpha(255)
		drystal.set_color(255, 255, 255)
		font:draw(text, W / 2, H / 2 - htext / 2, 2)

		smallfont:draw('Click to restart', W / 2, H * .7, 2)
	end
end

local function check_winner()
	for i = 1, 3 do
		if board[i][1] ~= 0 and board[i][1] == board[i][2] and board[i][2] == board[i][3] then
			return board[i][1]
		end
	end
	for j = 1, 3 do
		if board[1][j] ~= 0 and board[1][j] == board[2][j] and board[2][j] == board[3][j] then
			return board[1][j]
		end
	end
	if board[1][1] ~= 0 and board[1][1] == board[2][2] and board[2][2] == board[3][3] then
		return board[1][1]
	end
	if board[3][1] ~= 0 and board[3][1] == board[2][2] and board[2][2] == board[1][3] then
		return board[3][1]
	end
	return false
end

local function check_board_full()
	for i = 1, 3 do
		for j = 1, 3 do
			if board[i][j] == 0 then
				return false
			end
		end
	end
	return true
end

local function play()
	local i, j = highlighted_tile.i, highlighted_tile.j
	if i >= 1 and i <= 3
	and j >= 1 and j <= 3 then
		if board[i][j] ~= 0 then
			return
		end
		board[i][j] = current_player
		if check_winner() then
			ended = true
		elseif check_board_full() then
			ended = true
			current_player = 0
		else
			if current_player == 1 then
				current_player = 2
			else
				current_player = 1
			end
		end
	end
end

local function restart()
	ended = false
	current_player = 1
	board = {
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0},
	}
end

function drystal.mouse_motion(x, y)
	highlighted_tile = {
		i = math.ceil(x / W * 3),
		j = math.ceil(y / H * 3),
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
