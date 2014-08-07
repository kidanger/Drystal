local drystal = require 'drystal'

drystal.create_world(0, 0)

local shape = drystal.new_shape('circle', 5)
local body = drystal.new_body(true, shape)
local body2 = drystal.new_body(true, shape)
shape = nil
collectgarbage()

local joint = drystal.new_joint('distance', body, body2)

print(body:get_position())
body:destroy()
-- the joint should be destroyed yet

if pcall(body.get_position, body) then
	error("body:get_position should fail after body:destroy")
end
if pcall(joint.destroy, joint) then
	error("joint:destroy should fail after body:destroy")
end

body = nil
if pcall(joint.set_length, joint, 1) then
	error('joint:set_length should fail after body:destroy')
end

joint = nil -- no need to destroy it, body:destroy() did it
collectgarbage()

body2 = nil
if pcall(collectgarbage) then
	error("body2 = nil + collectgarbage should fail because body2 hasn't been destroyed")
end

print 'ending'
drystal.stop()

