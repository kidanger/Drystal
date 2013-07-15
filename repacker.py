INPUT = 'index_nodata'
OUTPUT = 'index'
DATADIR = 'data'

HTML = '.html'
JS = '.js'
DATA = '.data'
COMPRESS = '.compress'

PACKAGER = 'python2 /home/anger/dev/emtest/tools/file_packager.py'
COMPRESSOR = '/home/anger/dev/emtest/third_party/lzma.js/lzma-native'

START_TOKEN = "// {{PRE_RUN_ADDITIONS}}\n"
END_TOKEN = "if (Module['preInit']) {\n"

import os

def replace_in_file(filename_src, start_token, new_lines, end_token, filename_dst):
    lines = open(filename_src, 'r').readlines()
    start = lines.index(start_token) + 1
    end = lines.index(end_token)
    lines[start:end] = new_lines
    open(filename_dst, 'w').write("".join(lines))

def repack():
    tmp_file = 'preload.js'
    # compression has to be done by the packager
    cmd = PACKAGER + " " + OUTPUT+DATA + " --preload " + DATADIR + " --compress " + COMPRESSOR + " > " + tmp_file
    print(cmd)
    os.system(cmd)

    # update index.js
    print('composing ' + OUTPUT+JS + ' from ' + INPUT+JS + ' and ' + tmp_file)
    new_part = open(tmp_file).readlines()
    replace_in_file(INPUT+JS, START_TOKEN, new_part, END_TOKEN, OUTPUT+JS)

    # and recompress it
    cmd = COMPRESSOR + " < " + OUTPUT+JS + " > " + OUTPUT+JS+COMPRESS
    print(cmd)
    os.system(cmd)

    # create a new index.html (need to modify 'index_nodata' to 'index')
    input = open(INPUT+HTML).read()
    output = input.replace(INPUT, OUTPUT)
    open(OUTPUT+HTML, 'w').write(output)


# here, we should have :
# index_nodata.html, index_nodata.js,
# index_nodata.js.compress (not used, but generated to have compression handled)
repack()
