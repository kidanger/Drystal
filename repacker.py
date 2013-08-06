#!/usr/bin/env python3
# coding: utf-8

INPUT   = 'index_nodata'
OUTPUT  = 'index'
DATADIR = 'gamedata'

JS      = '.js'
HTML    = '.html'
DATA    = '.data'
COMPRESS= '.compress'

PACKAGER        = 'python2 /usr/lib/emscripten/tools/file_packager.py'
COMPRESSOR      = '/usr/lib/emscripten/third_party/lzma.js/lzma-native'
DECOMPRESS_JS   = '/usr/lib/emscripten/third_party/lzma.js/lzma-decoder.js'
DECOMPRESS_NAME = 'LZMA.decompress'

START_TOKEN = "// {{PRE_RUN_ADDITIONS}}\n"
END_TOKEN   = "if (Module['preInit']) {\n"

DO_CLOSURE = False

import os
import shutil

def replace_in_file(filename_src, start_token, new_lines, end_token, filename_dst):
    lines = open(filename_src, 'r').readlines()
    start = lines.index(start_token) + 1
    end = lines.index(end_token)
    lines[start:end] = new_lines
    open(filename_dst, 'w').write("".join(lines))

def repack(index_html_path):
    tmp_file = 'preload.js'

    # compression has to be done by the packager
    cmd = PACKAGER + " " + OUTPUT+DATA + " --preload " + DATADIR+"@/" + " --compress " + COMPRESSOR + " > " + tmp_file
    print(cmd)
    assert(not os.system(cmd))

    # update javascript
    print('composing ' + OUTPUT+JS + ' from ' + INPUT+JS + ' and ' + tmp_file)
    new_part = open(tmp_file).readlines()
    replace_in_file(INPUT+JS, START_TOKEN, new_part, END_TOKEN, OUTPUT+JS)
    if DO_CLOSURE:
        filename = OUTPUT+JS
        cmd = 'java -jar /usr/lib/emscripten/third_party/closure-compiler/compiler.jar ' +\
                '--compilation_level ADVANCED_OPTIMIZATIONS ' +\
                '--language_in ECMASCRIPT5 ' + \
                '--js ' + filename + ' --js_output_file ' + filename+'_' +\
                '; mv ' + filename+'_' + ' ' + filename
        print(cmd)
        assert(not os.system(cmd))

    # copy index.html
    print('copying index.html from', index_html_path)
    shutil.copy(index_html_path, OUTPUT+HTML)

    # create decompress.js (see emcc.py)
    print('generating decompress.js')
    decompressor = open('decompress.js', 'w')
    decompressor.write(open(DECOMPRESS_JS).read())
    decompressor.write('''
                       onmessage = function(event) {
                       postMessage({ data: %s(event.data.data), id: event.data.id });
                       };
                       ''' % DECOMPRESS_NAME)
    decompressor.close()

    # recompress javascript
    cmd = COMPRESSOR + " < " + OUTPUT+JS + " > " + OUTPUT+JS+COMPRESS
    print(cmd)
    assert(not os.system(cmd))

