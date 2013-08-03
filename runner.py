#!/usr/bin/env python3
# coding: utf-8

import sys
import os
import shutil

DESTINATION_DIRECTORY = os.path.abspath('data')
DESTINATION_DIRECTORY_TMP = os.path.abspath('.data')

BUILD_NATIVE = os.path.abspath('build-native')
BUILD_WEB_REL = 'build-web'
BUILD_WEB = os.path.abspath(BUILD_WEB_REL)

EXTENSIONS_DIRECTORY = os.path.abspath('extensions')
EXTENSIONS_DIRECTORY_NATIVE = os.path.join(BUILD_NATIVE, 'extensions')
EXTENSIONS_DIRECTORY_WEB = os.path.join(BUILD_WEB, 'extensions')

EXTENSIONS_NATIVE = 'main.so'
EXTENSIONS_WEB = 'main.so'

LIB_PATH = os.path.join(BUILD_NATIVE, 'external')

BROWSERS = 'chromium', 'firefox'

def parent(dir):
    return os.path.abspath(os.path.join(dir, os.pardir))

def copy_files_maybe(from_directory, get_subdir=False):
    print('- processing', from_directory)
    already_in = os.listdir(DESTINATION_DIRECTORY)
    for f in os.listdir(from_directory):
        if f in ('.', '..') or f.startswith('.'):
            continue
        if f not in already_in:
            fullpath = os.path.join(from_directory, f)
            if os.path.isfile(fullpath):
                print('    copying\t', f)
                shutil.copy(fullpath, DESTINATION_DIRECTORY)
            if os.path.isdir(fullpath) and get_subdir:
                print('    copying dir\t', f)
                shutil.copytree(fullpath, os.path.join(DESTINATION_DIRECTORY, f))
        else:
            print('    ignoring\t', f)

def copy_extensions(from_dir, ext_list, mainfilename):
    for extension in ext_list:
        src_path = os.path.join(from_dir,
                                extension,
                                mainfilename)
        dst_path = os.path.join(DESTINATION_DIRECTORY,
                                mainfilename.replace('main', extension))
        if os.path.exists(src_path):
            shutil.copy(src_path, dst_path)
            print('- add extension ', src_path)
        else:
            print('! extension not available: ', src_path)

def clean():
    if not os.path.exists(DESTINATION_DIRECTORY_TMP):
        return False
    shutil.rmtree(DESTINATION_DIRECTORY)
    shutil.copytree(DESTINATION_DIRECTORY_TMP, DESTINATION_DIRECTORY)
    shutil.rmtree(DESTINATION_DIRECTORY_TMP)
    return True

cw = os.getcwd()

main_arg = sys.argv[1]
run_arg = len(sys.argv) == 3 and sys.argv[2] or ''
if (len(sys.argv) < 2
    or not (os.path.exists(main_arg)
            or main_arg == 'clean')
    or not run_arg in ('', 'native', 'web')
        ):
    print('usage:', sys.argv[0], '<directory>[/filename.lua] [native|web]')
    print('      ', sys.argv[0], 'clean')
    sys.exit(1)

if sys.argv[1] == 'clean':
    if clean():
        print(DESTINATION_DIRECTORY, 'restored')
    else:
        print('directory isn\'t dirty')

else:
    if clean():
        print(DESTINATION_DIRECTORY, 'restored')

    to_be_run = main_arg

    if os.path.isdir(to_be_run):
        # in case the trailing '/' is omitted, because path.split wouldn't behave as we want
        dirpath = to_be_run
        file = None
    else:
        dirpath, file = os.path.split(to_be_run)

    # here, if file is not null, it will be renamed to 'main.lua'
    # if it's null, we asume there's already a 'main.lua' in dirpath
    main = os.path.join(DESTINATION_DIRECTORY, 'main.lua')

    dir = os.path.abspath(dirpath)

    shutil.copytree(DESTINATION_DIRECTORY, DESTINATION_DIRECTORY_TMP)
    if os.path.exists(main):
        os.remove(main)

    first = True
    while len(cw) < len(dir):
        copy_files_maybe(dir, get_subdir=first)
        dir = parent(dir)
        first = False

    if file and file != 'main.lua':
        print('rename', file, 'to main.lua')
        os.rename(os.path.join(DESTINATION_DIRECTORY, file), main)

    if run_arg == 'native':
#        copy_extensions(EXTENSIONS_DIRECTORY_NATIVE,
#                        [f for f in os.listdir(EXTENSIONS_DIRECTORY)
#                           if os.path.isdir(os.path.join(EXTENSIONS_DIRECTORY, f))],
#                        EXTENSIONS_NATIVE)

        os.environ['LD_LIBRARY_PATH'] = LIB_PATH
        os.system('./build-native/drystal')
    elif run_arg == 'web':
        # TODO extensions (wait for emscripten to support it)
        os.system('tup upd') # TODO: use repacker directly
        from http.server import HTTPServer, SimpleHTTPRequestHandler
        addr, port = '127.0.0.1', 8000
        httpd = HTTPServer((addr, port), SimpleHTTPRequestHandler)
        for b in BROWSERS:
            if not os.system(b + ' ' + addr + ':' + str(port) + '/' + BUILD_WEB_REL + ' >/dev/null'):
                print('- page opened in', b)
                break
        else:
            print('! unable to open a browser')
        httpd.serve_forever()



