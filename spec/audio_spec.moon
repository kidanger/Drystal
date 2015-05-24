drystal = require 'drystal'

describe 'audio', ->
	describe 'music', ->
		it 'loads ogg', ->
			with m = drystal.load_music 'tests/audio/test.ogg'
				assert.not_nil m

		it 'returns an error if the file does not exist', ->
			with ok, err = drystal.load_music 'does_not_exist.ogg'
				assert.nil ok
				assert.string err

		it 'returns an error if the file is not an ogg', ->
			with ok, err = drystal.load_music 'spec/surface_spec.moon'
				assert.nil ok
				assert.string err
			with ok, err = drystal.load_music 'tests/audio/test.wav'
				assert.nil ok
				assert.string err

	describe 'sound', ->
		it 'loads wav', ->
			with s = drystal.load_sound 'tests/audio/test.wav'
				assert.not_nil s

		it 'returns an error if the file does not exist', ->
			with ok, err = drystal.load_sound 'does_not_exist.wav'
				assert.nil ok
				assert.string err

		it 'returns an error if the file is not an wav', ->
			with ok, err = drystal.load_sound 'spec/surface_spec.moon'
				assert.nil ok
				assert.string err
			with ok, err = drystal.load_sound 'tests/audio/test.ogg'
				assert.nil ok
				assert.string err

