local drystal = drystal
function drystal.init()
	drystal.resize(100, 100)
end
function drystal.resize_event(w, h)
	print("should not be printed")
end
