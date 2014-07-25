local drystal = require 'drystal'

drystal.run_js [[
	document.title = "Drystal js.lua";
	document.body.style.backgroundColor = "red";
	window.alert("hello from drystal");
]]

drystal.stop()

