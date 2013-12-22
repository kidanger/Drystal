#!/usr/bin/env python3
# coding: utf-8

import sys
import os
import shutil
import subprocess
import argparse
import signal

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

BINARY_DIRECTORY_NATIVE = os.path.join(BUILD_NATIVE, 'src')
EXTENSIONS_DIRECTORY = os.path.abspath('extensions')
EXTENSIONS_DIRECTORY_NATIVE = os.path.join(BUILD_NATIVE, 'extensions')
EXTENSIONS_DIRECTORY_WEB = os.path.join(BUILD_WEB, 'extensions')

EMSCRIPTEN_ROOT_PATH='/usr/lib/emscripten'
EMSCRIPTEN_CMAKE_DEFINES = ['CMAKE_TOOLCHAIN_FILE=../cmake/Emscripten.cmake',
                            'EMSCRIPTEN_ROOT_PATH=' + EMSCRIPTEN_ROOT_PATH,
                            'EMSCRIPTEN=1']
LIB_PATH = os.path.join(BUILD_NATIVE, 'external')
VALGRIND_ARGS = '--tool=callgrind'

BROWSERS = 'chromium', 'firefox'

WGET_FILES = []
IGNORE_FILES = ['index.html', 'drystal.cfg']
SUBDIRS = []

HAS_NINJA = subprocess.call(['which', 'ninja'], stdout=subprocess.DEVNULL) == 0

def parent(directory):
    return os.path.abspath(os.path.join(directory, os.pardir))

def execute(args, fork=False, cwd='.'):
    print(I, ' '.join(args), 'from', cwd, N)
    if fork:
        return subprocess.Popen(args, cwd=cwd)
    else:
        return subprocess.call(args, cwd=cwd)

def has_been_modified(fullpath, old):
    if not os.path.exists(old):
        return True
    elif os.path.isdir(fullpath):
        for f in os.listdir(fullpath):
            o = os.path.join(old, f)
            fp = os.path.join(fullpath, f)
            if has_been_modified(fp, o):
                return True
    elif os.path.getmtime(fullpath) > os.path.getmtime(old):
        return True
    return False

def copy_files_maybe(from_directory, get_subdir=False, verbose=True):
    if verbose:
        _print = print
    else:
        _print = lambda *args, **kargs: None

    _print(G, '- processing', from_directory, N)
    did_copy = False
    for f in os.listdir(from_directory):
        if f.startswith('.') or f in IGNORE_FILES:
            _print(I, '    ignoring\t', f)
            continue
        old = os.path.join(DESTINATION_DIRECTORY, f)
        fullpath = os.path.join(from_directory, f)
        if os.path.isdir(fullpath) and (get_subdir or f in SUBDIRS) and has_been_modified(fullpath, old):
            _print(G, '    copying dir\t', f)
            if os.path.exists(old):
                shutil.rmtree(old)
            shutil.copytree(fullpath, old)
            did_copy = True
        elif os.path.isfile(fullpath) and has_been_modified(fullpath, old):
            if os.path.splitext(fullpath)[1] not in IGNORE_FILES:
                print(G, '    copying\t', f)
                shutil.copy(fullpath, DESTINATION_DIRECTORY)
                did_copy = True
            else:
                _print(I, '    ignoring ext', f)
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
    print(G, '- processing for wget: ', from_directory, 'to', destination, N)
    if not os.path.exists(destination):
        os.mkdir(destination)
    for f in os.listdir(from_directory):
        if f in ('.', '..') or f.startswith('.'):
            continue
        fullpath = os.path.join(from_directory, f)
        if os.path.isfile(fullpath) and os.path.splitext(fullpath)[1] in WGET_FILES:
            print(G, '    wget\t', f)
            shutil.move(fullpath, destination)

def copy_extensions(from_dir, ext_list):
    for extension in ext_list:
        src_path = os.path.join(from_dir,
                                extension,
                                'lib'+extension+'.so')
        dst_path = os.path.join(DESTINATION_DIRECTORY,
                                extension + '.so')
        if os.path.exists(src_path):
            shutil.copy(src_path, dst_path)
            print(G, '- add extension ', src_path)
        else:
            print(G, '! extension not available: ', src_path)

def locate_index_html(from_dir, to_dir):
    directory = from_dir
    while directory != to_dir:
        files = os.listdir(directory)
        if 'index.html' in files:
            return os.path.join(directory, 'index.html')
        directory = parent(directory)
    return None

def clean(directory):
    return os.path.exists(directory) and shutil.rmtree(directory)

def run_clean():
    if clean(DESTINATION_DIRECTORY):
        print(G, '-', DESTINATION_DIRECTORY, 'deleted', N)
    else:
        print(E, 'directory isn\'t dirty', N)
    webdata = os.path.join(BUILD_WEB, DESTINATION_DIRECTORY_REL)
    if clean(webdata):
        print(G, '-', webdata, 'deleted', N)

def tup_update(build=''):
    if '.tup' not in os.listdir('.'):
        execute(['tup', 'init'])
    if execute(['tup', 'upd', build]) != 0:
        print(E, 'compilation failed, stopping.', N)
        sys.exit(1)

def cmake_update(build, definitions=[]):
    generator = HAS_NINJA and 'Ninja' or 'Unix Makefiles'
    compiler = HAS_NINJA and 'ninja' or 'make'
    if build not in os.listdir('.'):
        os.mkdir(build)
        defs = ['-D' + d for d in definitions]
        if execute(['cmake', '..', '-G', generator] + defs, cwd=build) != 0:
            print(E, 'cmake failed, stopping. remove', build, 'and try again', N)
            sys.exit(1)
    if execute([compiler], cwd=build) != 0:
        print(E, compiler, 'failed, stopping.', N)
        sys.exit(1)

def create_token(token):
    if not os.path.exists(DESTINATION_DIRECTORY):
        print(G, '- create', DESTINATION_DIRECTORY)
        os.mkdir(DESTINATION_DIRECTORY)
    print(W, '- add token', token)
    open(token, 'w').close()

def check_token(path):
    token = os.path.join(DESTINATION_DIRECTORY, '.' + path.replace('/', '\\'))
    if not os.path.exists(token):
        run_clean()
        create_token(token)
    else:
        print(W, '- token exists, don\'t clean')

def split_path(path):
    # here, if file is not null, it will be renamed to 'main.lua'
    # if it's null, we asume there's already a 'main.lua' in directory
    if os.path.isdir(path):
        # in case the trailing '/' is omitted, because path.split wouldn't behave as we want
        dirpath = path
        file = None
    else:
        dirpath, file = os.path.split(path)
    return os.path.abspath(dirpath), file

def prepare_data(path):
    check_token(path)
    directory, file = split_path(path)

    load_config(directory)
    copy_files(directory, file)
    return directory, file

def prepare_native():
    cmake_update('build-native')
    copy_extensions(EXTENSIONS_DIRECTORY_NATIVE,
                    [f for f in os.listdir(EXTENSIONS_DIRECTORY)
                       if os.path.isdir(os.path.join(EXTENSIONS_DIRECTORY, f))])

    os.chdir(DESTINATION_DIRECTORY)
    os.environ['LD_LIBRARY_PATH'] = LIB_PATH
    program = os.path.join(BINARY_DIRECTORY_NATIVE, 'drystal')
    return program

def run_repack(args):
    directory, file = prepare_data(args.PATH)
    cmake_update('build-web', EMSCRIPTEN_CMAKE_DEFINES)
    remove_old_wget()
    move_wget_files(DESTINATION_DIRECTORY, os.path.join(BUILD_WEB, DESTINATION_DIRECTORY_REL))
    htmlfile = locate_index_html(os.path.abspath(directory), os.getcwd())
    import repacker
    os.chdir(BUILD_WEB)
    repacker.DATADIR = os.path.join('..', repacker.DATADIR)
    repacker.INPUT = 'src/drystal'
    repacker.repack(htmlfile)
    os.chdir('..')

def run_web(args):
    run_repack(args)
    from http.server import HTTPServer, SimpleHTTPRequestHandler
    addr, port = '127.0.0.1', 8000
    httpd = HTTPServer((addr, port), SimpleHTTPRequestHandler)
    for b in BROWSERS:
        if not execute([b, addr + ':' + str(port) + '/' + BUILD_WEB_REL]):
            print(G, '- page opened in', b, N)
            break
        else:
            print(W, '! unable to open a browser', N)
    httpd.serve_forever()

def get_gdb_args(program, pid=None):
    # if debug, run with 'gdb' (and give it path to sources)
    source_directories = ['src', os.path.join('external', 'lua', 'src')]
    for f in os.listdir(EXTENSIONS_DIRECTORY):
        path = os.path.join(EXTENSIONS_DIRECTORY, f)
        if os.path.isdir(path):
            source_directories.append(path[path.find('extensions'):])
    args = ['gdb']
    for d in source_directories:
        args.append('-d')
        args.append(os.path.join('..', d))
    if pid:
        args += [program, str(pid), '-ex', 'handle SIGUSR1 ignore']
    else:
        args += ['-ex', 'run', program]
    return args

def setup_live_coding(directory, file, drystal):
    print(G, '- settings up live coding', N)
    import time
    try:
        while True:
            time.sleep(1)
            c = copy_files(directory, file, verbose=False)
            if c:
                drystal.send_signal(signal.SIGUSR1)
    except KeyboardInterrupt:
        drystal.terminate()
        sys.exit(1)

def run_native(args):
    directory, file = prepare_data(args.PATH)
    program = prepare_native()
    if args.debug:
        if args.live:
            drystal = execute([program], fork=True)
            execute(get_gdb_args(program, drystal.pid), fork=True)
        else:
            execute(get_gdb_args(program), fork=False)
    elif args.profile:
        drystal = execute(['valgrind', VALGRIND_ARGS, program], fork=args.live)
    else:
        drystal = execute([program], fork=args.live)

    if args.live:
        setup_live_coding(directory, file, drystal)

def copy_files(directory, file, verbose=True):
    has_copied_some_files = copy_files_maybe(INCLUDE_DIRECTORY, get_subdir=True, verbose=verbose)
    has_copied_some_files = copy_files_maybe(directory, get_subdir=True, verbose=verbose) or has_copied_some_files

    main = os.path.join(DESTINATION_DIRECTORY, 'main.lua')
    mainfile = file or 'main.lua'
    notmain = os.path.join(DESTINATION_DIRECTORY, mainfile)
    if (has_copied_some_files or not os.path.exists(main) or os.path.getmtime(notmain) > os.path.getmtime(main)):
        if notmain != main:
            print(W, '- rename', file, 'to main.lua', N)
            shutil.copy(notmain, main)
        else:
            print(W, '- touch', main, N)
            os.utime(main, None)
    return has_copied_some_files

def valid_path(path):
    if not os.path.exists(path):
        msg = "%r does not exist" % path
        raise argparse.ArgumentTypeError(msg)
    return path

parser = argparse.ArgumentParser(description='Drystal roadrunner !')
subparsers = parser.add_subparsers(help='sub-commands')

parser_native = subparsers.add_parser('native', help='run with drystal', description='run with drystal')
parser_native.add_argument('PATH', help='<directory>[/filename.lua]', type=valid_path)
parser_native.set_defaults(func=run_native)
parser_native.add_argument('-l', '--live', help='live coding (reload code when it has been modified)', action='store_true', default=False)
group = parser_native.add_mutually_exclusive_group()
group.add_argument('-d', '--debug', help='debug with gdb', action='store_true', default=False)
group.add_argument('-p', '--profile', help='profile with valgrind', action='store_true', default=False)

parser_web = subparsers.add_parser('web', help='run in a browser', description='run in a browser')
parser_web.add_argument('PATH', help='<directory>[/filename.lua]', type=valid_path)
parser_web.set_defaults(func=run_web)

parser_repack = subparsers.add_parser('repack', help='repack', description='repack')
parser_repack.add_argument('PATH', help='<directory>[/filename.lua]', type=valid_path)
parser_repack.set_defaults(func=run_repack)

parser_clean = subparsers.add_parser('clean', help='cleanup gamedata/', description='cleanup gamedata/')
parser_clean.set_defaults(func=run_clean)

if len(sys.argv) > 1:
    args = parser.parse_args()
    args.func(args)
else:
    parser.print_usage()
