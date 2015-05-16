drystal = require 'drystal'

describe 'buffer', ->

    it 'cannot be modified after free', ->
        buffer = drystal.new_buffer!
        buffer\upload_and_free!
        assert.error -> buffer\reset!

    it 'resizes', ->
        buffer = drystal.new_buffer 10
        buffer\use!
        drystal.draw_line 0, 0, 0, 0 for i = 0, 50

