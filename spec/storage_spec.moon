drystal = require 'drystal'

describe 'storage', ->

	it 'stores data', ->
		drystal.store 'test_storage', {text:'blabla', duck:58}
		data = drystal.fetch 'test_storage'
		assert.same {text:'blabla', duck:58}, data

	it 'overwrites previous stored data', ->
		drystal.store 'test_storage', {text:'blabla', duck:58}
		drystal.store 'test_storage', {text:'foobar', duck:59}
		data = drystal.fetch 'test_storage'
		assert.same {text:'foobar', duck:59}, data

	it 'does not contain unknown keys', ->
		data = drystal.fetch 'unknown'
		assert.nil data

