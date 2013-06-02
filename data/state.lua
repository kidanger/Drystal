local state = {
}

local Machine = {}
Machine.__index = Machine

function Machine.new()
	local machine = {
		states = {},
		current = nil,
	}

	return setmetatable(machine, Machine)
end

function Machine:push(state)
	state.parent = self.current
	self:switch_state(state)
end

function Machine:pop()
	new = self.current and self.current.parent
	if new == nil then
		print ("No state to pop, stop engine")
		engine_stop()
	end
	self:switch_state(new)
	return new
end

function Machine:call(f, ...)
	args = {...}
	if self.current and self.current[f] then
		self.current[f](self.current, unpack(args))
	else
		--print (f .. ' not called on ' .. (self.current and unpack(self.current) or 'nil'))
	end
end

function Machine:switch_state(state)
	if self.current and self.current.on_exit then
		self.current:on_exit()
	end
	self.current = state
	if self.current and self.current.on_enter then
		self.current:on_enter()
	end
end

return Machine:new()
