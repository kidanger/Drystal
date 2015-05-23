#!/usr/bin/env drystal

package.path = './.rocks/share/lua/5.3/?.lua;./.rocks/share/lua/5.3/?/init.lua;' .. package.path
package.cpath = './.rocks/lib/lua/5.3/?.so;' .. package.cpath

require "moonscript"
require 'spec.tools'
drystal = require 'drystal'
os.exit = drystal.stop

require 'busted.runner' {
	batch=true,
}

