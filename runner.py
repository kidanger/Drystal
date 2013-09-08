#!/usr/bin/env python3
# coding: utf-8

import sys
import os
import shutil

G = '\033[92m'
I = '\033[95m'
W = '\033[93m'
E = '\033[91m'
N = '\033[m'

DESTINATION_DIRECTORY_REL = 'gamedata'
DESTINATION_DIRECTORY = os.path.abspath(DESTINATION_DIRECTORY_REL)
INCLUDE_DIRECTORY = os.path.abspath('data')

BUILD_NATIVE = os.path.abspath('build-native')
BUILD_WEB_REL = 'build-web'
BUILD_WEB = os.path.abspath(BUILD_WEB_REL)

EXTENSIONS_DIRECTORY = os.path.abspath('extensions')
EXTENSIONS_DIRECTORY_NATIVE = os.path.join(BUILD_NATIVE, 'extensions')
EXTENSIONS_DIRECTORY_WEB = os.path.join(BUILD_WEB, 'extensions')

EXTENSIONS_NATIVE = 'main.so'
EXTENSIONS_WEB = 'main.so'

LIB_PATH = os.path.join(BUILD_NATIVE, 'external')
VALGRIND_ARGS = '--tool=callgrind'

BROWSERS = 'chromium', 'firefox'

WGET_FILES = []
IGNORE_FILES = ['index.html', 'drystal.cfg']
SUBDIRS = []

def parent(dir):
    return os.path.abspath(os.path.join(dir, os.pardir))

def copy_files_maybe(from_directory, get_subdir=False, verbose=True):
    if verbose:
        _print = print
    else:
        _print = lambda *args, **kargs: None

    _print(G, '- processing', from_directory)
    did_copy = False
    for f in os.listdir(from_directory):
        if f.startswith('.'):
            continue
        if f in IGNORE_FILES:
            _print(I, '    ignoring\t', f)
            continue
        old = os.path.join(DESTINATION_DIRECTORY, f)
        fullpath = os.path.join(from_directory, f)
        if not os.path.exists(old) or os.path.getmtime(fullpath) > os.path.getmtime(old):
            if os.path.isfile(fullpath):
                if os.path.splitext(fullpath)[1] not in IGNORE_FILES:
                    print(G, '    copying\t', f)
                    shutil.copy(fullpath, DESTINATION_DIRECTORY)
                    did_copy = True
                else:
                    _print(I, '    ignoring ext', f)
            if os.path.isdir(fullpath) and (get_subdir or f in SUBDIRS):
                _print(G, '    copying dir\t', f)
                shutil.copytree(fullpath, os.path.join(DESTINATION_DIRECTORY, f))
                did_copy = True
        else:
            _print(I, '    already\t', f)
    return did_copy

def remove_old_wget():
    destination = os.path.join(BUILD_WEB, DESTINATION_DIRECTORY_REL)
    if not os.path.exists(destination):
        return
    print(G, '- remove old wget: ', destination)
    for f in os.listdir(destination):
        fullpath = os.path.join(destination, f)
        if os.path.isfile(fullpath):
            os.remove(fullpath)
        else:
            shutil.rmtree(fullpath)


def load_config(from_directory):
    import json
    cfg = os.path.join(from_directory, 'drystal.cfg')
    if os.path.exists(cfg):
        print(G, '- reading configuration from', cfg)
        config = json.load(open(cfg))
        global WGET_FILES, IGNORE_FILES, SUBDIRS
        WGET_FILES += 'wget' in config and config['wget'] or []
        IGNORE_FILES += 'ignore' in config and config['ignore'] or []
        SUBDIRS += 'subdirs' in config and config['subdirs'] or []

def move_wget_files(from_directory, destination):
    print(G, '- processing for wget: ', from_directory, 'to', destination)
    if not os.path.exists(destination):
        os.mkdir(destination)
    for f in os.listdir(from_directory):
        if f in ('.', '..') or f.startswith('.'):
            continue
        fullpath = os.path.join(from_directory, f)
        if os.path.isfile(fullpath) and os.path.splitext(fullpath)[1] in WGET_FILES:
            print(G, '    wget\t', f)
            shutil.move(fullpath, destination)

def copy_extensions(from_dir, ext_list, mainfilename):
    for extension in ext_list:
        src_path = os.path.join(from_dir,
                                extension,
                                mainfilename)
        dst_path = os.path.join(DESTINATION_DIRECTORY,
                                mainfilename.replace('main', extension))
        if os.path.exists(src_path):
            shutil.copy(src_path, dst_path)
            print(G, '- add extension ', src_path)
        else:
            print(G, '! extension not available: ', src_path)

def locate_index_html(from_dir, to_dir):
    dir = from_dir
    while dir != to_dir:
        files = os.listdir(dir)
        if 'index.html' in files:
            return os.path.join(dir, 'index.html')
        dir = parent(dir)
    return None

def clean(dir):
    cleaned = False
    if os.path.exists(dir):
        cleaned = True
        shutil.rmtree(dir)
    return cleaned

def clean_all():
    if clean(DESTINATION_DIRECTORY):
        print(G, '-', DESTINATION_DIRECTORY, 'deleted', N)
    else:
        print(E, 'directory isn\'t dirty', N)
    webdata = os.path.join(BUILD_WEB, DESTINATION_DIRECTORY_REL)
    if clean(webdata):
        print(G, '-', webdata, 'deleted', N)

cw = os.getcwd()

main_arg = len(sys.argv) > 1 and sys.argv[1] or ''
run_arg = len(sys.argv) == 3 and sys.argv[2] or ''

if (len(sys.argv) < 2
    or not (os.path.exists(main_arg)
            or main_arg == 'clean')
    or not run_arg in ('', 'native', 'live', 'debug', 'profile', 'web', 'repack',)
        ):
    print('usage:', sys.argv[0], '<directory>[/filename.lua] [native|live|debug|profile|web|repack]')
    print('      ', sys.argv[0], 'clean')
    sys.exit(1)

if sys.argv[1] == 'clean':
    clean_all()

else:
    to_be_run = main_arg

    token = os.path.join(DESTINATION_DIRECTORY, '.' + to_be_run.replace('/', '\\'))
    if not os.path.exists(token):
        clean_all()
    else:
        print(W, '- token exists, don\'t clean')

    if not os.path.exists(DESTINATION_DIRECTORY):
        print(G, '- create', DESTINATION_DIRECTORY)
        os.mkdir(DESTINATION_DIRECTORY)
        print(W, '- add token', token)
        open(token, 'w').close()

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

    load_config(dir)

    def copy_files(verbose=True):
        has_copied_some_files = False # main.lua is over
        dir = os.path.abspath(dirpath)
        has_copied_some_files = copy_files_maybe(INCLUDE_DIRECTORY, get_subdir=True, verbose=verbose) or has_copied_some_files

        first = True
        while cw != dir:
            last = parent(dir) == cw
            has_copied_some_files = copy_files_maybe(dir, get_subdir=first and not last, verbose=verbose) or has_copied_some_files
            dir = parent(dir)
            first = False

        mainfile = file or 'main.lua'
        notmain = os.path.join(DESTINATION_DIRECTORY, mainfile)
        if (has_copied_some_files
        or not os.path.exists(main) or os.path.getmtime(notmain) > os.path.getmtime(main)):
            if notmain != main:
                print(W, '- rename', file, 'to main.lua', N)
                shutil.copy(notmain, main)
            else:
                print(W, '- touch', main, N)
                os.utime(main, None)

    copy_files()

    if run_arg in ('native', 'live', 'debug', 'profile'):
        if '.tup' in os.listdir('.'):
            assert(not os.system('tup upd build-native'))
        copy_extensions(EXTENSIONS_DIRECTORY_NATIVE,
                        [f for f in os.listdir(EXTENSIONS_DIRECTORY)
                           if os.path.isdir(os.path.join(EXTENSIONS_DIRECTORY, f))],
                        EXTENSIONS_NATIVE)

        os.environ['LD_LIBRARY_PATH'] = LIB_PATH
        program = '../build-native/drystal'
        # if debug, run with 'gdb' (and give it path to sources)
        if run_arg == 'debug':
            source_directories = ['src', 'external/lua/src']
            for f in os.listdir(EXTENSIONS_DIRECTORY):
                path = os.path.join(EXTENSIONS_DIRECTORY, f)
                if os.path.isdir(path):
                    source_directories.append(path[len(os.getcwd()) + 1:])
            source_directories = map(lambda d: '-d ../' + d, source_directories)
            program = 'gdb ' + ' '.join(source_directories) + ' -ex run ' + program
        if run_arg == 'profile':
            program = 'valgrind ' + VALGRIND_ARGS + ' ' + program
        if run_arg == 'live':
            program += ' &'
        cmd = 'cd ' + DESTINATION_DIRECTORY + '; ' + program
        print(I, cmd, N)
        os.system(cmd)
        if run_arg == 'live':
            print(G, '- settings up live coding', N)
            import time
            try:
                while True:
                    copy_files(verbose=False)
                    time.sleep(1)
            except KeyboardInterrupt:
                print(I, 'kill drystal', N)
                os.system('pkill drystal')
                sys.exit()

    elif run_arg in ('web', 'repack'):
        if '.tup' in os.listdir('.'):
            assert(not os.system('tup upd build-web'))
        remove_old_wget()
        move_wget_files(DESTINATION_DIRECTORY, os.path.join(BUILD_WEB, DESTINATION_DIRECTORY_REL))
        htmlfile = locate_index_html(os.path.abspath(dirpath), os.getcwd())
        import repacker
        os.chdir(BUILD_WEB)
        repacker.DATADIR = '../' + repacker.DATADIR
        repacker.repack(htmlfile)
        os.chdir('..')

        if run_arg == 'web':
            from http.server import HTTPServer, SimpleHTTPRequestHandler
            addr, port = '127.0.0.1', 8000
            httpd = HTTPServer((addr, port), SimpleHTTPRequestHandler)
            for b in BROWSERS:
                if not os.system(b + ' ' + addr + ':' + str(port) + '/' + BUILD_WEB_REL + ' >/dev/null'):
                    print(G, '- page opened in', b)
                    break
            else:
                print(W, '! unable to open a browser')
            httpd.serve_forever()



