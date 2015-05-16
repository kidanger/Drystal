drystal = require 'drystal'

describe 'buffer', ->

    it 'cannot be modified after free', ->
        buffer = drystal.new_buffer!
        buffer\upload_and_free!
        assert.error -> buffer\reset!
        assert.error -> buffer\use!
        assert.error -> buffer\upload_and_free!

    it 'resizes', ->
        buffer = drystal.new_buffer 10
        buffer\use!
        drystal.draw_line 0, 0, 0, 0 for i = 0, 50

    it 'should not accept invalid sizes', ->
        assert.error -> drystal.new_buffer -1
        assert.error -> drystal.new_buffer 0

    -- TODO: drawing

