local drystal = drystal

print 'should print: Exiting'
function drystal.init()
	print 'should not be printed'
end

function drystal.update(dt)
end

function drystal.draw()
end

function drystal.atexit()
	print 'Exiting'
end

drystal.stop()

