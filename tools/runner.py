#!/usr/bin/env python3
# coding: utf-8
#
# This is a drystal developer tool, it is not intended to help game developpers.

import os
import sys
import shutil
import fnmatch
import zipfile
import subprocess
import configparser
import signal

G = '\033[92m'
I = '\033[95m'
W = '\033[93m'
E = '\033[91m'
N = '\033[m'

BUILD_NATIVE_RELEASE = os.path.abspath('build-native-release')
BUILD_NATIVE_DEBUG = os.path.abspath('build-native-debug')
BUILD_WEB = os.path.abspath('build-web')

join = os.path.join

BINARY_DIRECTORY_NATIVE_RELEASE = join(BUILD_NATIVE_RELEASE, 'src')
BINARY_DIRECTORY_NATIVE_DEBUG = join(BUILD_NATIVE_DEBUG, 'src')
BINARY_DIRECTORY_NATIVE_WEB = join(BUILD_WEB, 'src')

NATIVE_CMAKE_DEFINES = []

EMSCRIPTEN_ROOT_PATH = '/usr/lib/emscripten'
EMSCRIPTEN_CMAKE_DEFINES = ['CMAKE_TOOLCHAIN_FILE=../cmake/Emscripten.cmake',
                            'EMSCRIPTEN_ROOT_PATH=' + EMSCRIPTEN_ROOT_PATH,
                            'EMSCRIPTEN=1',
                            'BUILD_LIVECODING=NO',
                            'CMAKE_BUILD_TYPE=Release']

LIB_PATH_RELEASE = join(BUILD_NATIVE_RELEASE, 'external')
LIB_PATH_DEBUG = join(BUILD_NATIVE_DEBUG, 'external')
VALGRIND_ARGS_MEMCHECK = ['--tool=memcheck', '--leak-check=full', '--suppressions=' + os.path.abspath('./tools/drystal.supp')]
VALGRIND_ARGS_PROFILE = ['--tool=callgrind']

BROWSERS = 'chromium', 'firefox'

HAS_NINJA = subprocess.call(['which', 'ninja'], stdout=subprocess.DEVNULL) == 0

DRYSTAL_LOAD = '<script type="text/javascript" src="drystal.js"></script>'

DRYSTAL_ADD_ARGUMENTS = '''
<script type='text/javascript'>
Module[\'arguments\'] = ARGS;
</script>
'''

def signal_handler(signum, frame):
    print(I + 'Signal ' + str(signum) + ' caught, quitting...')
    exit(0)

def add_signal_handlers():
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGQUIT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)


def parent(directory):
    return os.path.abspath(join(directory, os.pardir))


def execute(args, fork=False, cwd=None, stdin=None, stdout=None):
    strcmd = ' '.join(args)
    if stdin:
        strcmd += ' < ' + stdin
        stdin = open(stdin, 'r')
    if stdout:
        strcmd += ' > ' + stdout
        stdout = open(stdout, 'w')
    if cwd:
        strcmd = '[' + cwd + '] $ ' + strcmd
    else:
        strcmd = '$ ' + strcmd

    print(I + strcmd, N)
    if fork:
        return subprocess.Popen(args, cwd=cwd, stdin=stdin, stdout=stdout)
    else:
        return subprocess.call(args, cwd=cwd, stdin=stdin, stdout=stdout)


def has_been_modified(fullpath, old):
    if not os.path.exists(old):
        return True
    elif os.path.isdir(fullpath):
        for f in os.listdir(fullpath):
            o = join(old, f)
            fp = join(fullpath, f)
            if has_been_modified(fp, o):
                return True
    elif os.path.getmtime(fullpath) > os.path.getmtime(old):
        return True
    return False


def load_config(from_directory):
    config = configparser.ConfigParser(allow_no_value=True)
    config.optionxform = str  # upper case is important too
    cfg = locate_recursively(os.path.abspath(from_directory),
                             os.getcwd(), 'drystal.cfg')
    if not cfg:
        print(E, 'cannot find drystal.cfg', N)
        sys.exit(1)
    print(G, '- reading configuration from', cfg)
    config.read(cfg)
    return config


def config_include_directory(config, directory):
    for rule in config:
        if rule != '*':
            if fnmatch.fnmatch(directory, rule):
                return True
    return directory in config


def config_is_wgetted(config, directory, file):
    if directory != '*' and config_is_wgetted(config, '*', file):
        return True
    if directory == '':
        directory = '.'
    if directory not in config or 'wget' not in config[directory]:
        return False
    for rule in config[directory]['wget'].split():
        if fnmatch.fnmatch(file, rule):
            return True
    return False


def config_include_file(config, directory, file):
    # magic directory
    if directory != '*' and config_include_file(config, '*', file):
        return True
    if directory == '':
        directory = '.'
    if not config_include_directory(config, directory):
        return False
    for rule in config[directory]:
        if fnmatch.fnmatch(file, rule):
            return True
    return False


def copy_wget_files(path, config, destination, verbose=False):
    print(G, '- copying wgot files', N)
    files = []

    def collect(path, directory):
        dirs = []
        for f in os.listdir(join(path, directory)):
            if f.startswith('.'):
                continue
            dest = join(directory, f)
            full = join(path, dest)
            if os.path.isdir(full) and config_include_directory(config, dest):
                dirs.append(dest)
            elif os.path.isfile(full):
                if config_is_wgetted(config, directory, f):
                    files.append(dest)
        for d in dirs:
            collect(path, d)

    collect(path, '')

    for f in files:
        dir = join(destination, os.path.split(f)[0])
        if not os.path.exists(dir):
            os.makedirs(dir)

        if verbose:
            print(G, '\t@ ', f, N)
        shutil.copy(join(path, f), join(destination, f))


def locate_recursively(from_dir, to_dir, name):
    directory = from_dir
    while directory != to_dir and directory != '/':
        files = os.listdir(directory)
        if name in files:
            return join(directory, name)
        directory = parent(directory)
    return None


def clean(directory):
    if not os.path.exists(directory):
        return False
    shutil.rmtree(directory)
    return True


def cmake_update(build, definitions=[], force_clean=False):
    generator = HAS_NINJA and 'Ninja' or 'Unix Makefiles'
    builder = HAS_NINJA and 'ninja' or 'make'
    if force_clean:
        clean(build)
    if not os.path.exists(build):
        os.mkdir(build)
        defs = ['-D' + d for d in definitions]
        if execute(['cmake', '..', '-G', generator] + defs, cwd=build) != 0:
            print(E, 'cmake failed. Fix CMakeLists.txt and try again!', N)
            clean(build)
            sys.exit(1)
    if execute([builder], cwd=build) != 0:
        print(E, builder, 'failed, stopping.', N)
        sys.exit(1)


def run_target(build, target):
    builder = HAS_NINJA and 'ninja' or 'make'
    if execute([builder, target], cwd=build) != 0:
        print(E, builder, target, 'failed, stopping.', N)
        sys.exit(1)


def prepare_native(release=False,enable_coverage=False):
    directory = ''
    build_type = ''
    lib_path = ''
    bin_path = ''
    coverage = ''
    if release:
        directory = BUILD_NATIVE_RELEASE
        build_type = 'Release'
        lib_path = LIB_PATH_RELEASE
        bin_path = BINARY_DIRECTORY_NATIVE_RELEASE
    else:
        directory = BUILD_NATIVE_DEBUG
        build_type = 'Debug'
        lib_path = LIB_PATH_DEBUG
        bin_path = BINARY_DIRECTORY_NATIVE_DEBUG

    if enable_coverage:
        coverage = 'ON'
    else:
        coverage = 'OFF'

    cmake_update(directory, ['CMAKE_BUILD_TYPE=' + build_type, 'BUILD_ENABLE_COVERAGE='+coverage] + NATIVE_CMAKE_DEFINES)
    if enable_coverage:
        run_target(directory, 'coverage-reset')

    os.environ['LD_LIBRARY_PATH'] = lib_path
    program = join(bin_path, 'drystal')
    return program, []


def prepare_drystaljs(destination):
    '''
        copy build-web/src/drystal.js to web/drystal.js
    '''
    srcjs = join(BINARY_DIRECTORY_NATIVE_WEB, 'drystal.js')
    js = join(destination, 'drystal.js')
    memsrcjs = join(BINARY_DIRECTORY_NATIVE_WEB, 'drystal.js.mem')
    memjs = join(destination, 'drystal.js.mem')

    if has_been_modified(srcjs, js):
        print(G, '- copy drystal.js', N)
        shutil.copyfile(srcjs, js)
    if os.path.exists(memsrcjs):
        if has_been_modified(memsrcjs, memjs):
            print(G, '- copy drystal.js.mem', N)
            shutil.copyfile(memsrcjs, memjs)


def package_data(path, zipname, destination, config, verbose=False):
    files = []

    def collect(path, directory):
        dirs = []
        for f in os.listdir(join(path, directory)):
            if f.startswith('.'):
                continue
            dest = join(directory, f)
            full = join(path, dest)
            if os.path.isdir(full) and config_include_directory(config, dest):
                dirs.append(dest)
            elif os.path.isfile(full):
                if config_include_file(config, directory, f):
                    if verbose:
                        print(G, '\t+ ', dest, N)
                    files.append((full, dest))
                else:
                    if verbose:
                        print(W, '\t~ ', dest, N)

        # collect directories after files
        for d in dirs:
            collect(path, d)

    if verbose:
        print(G, '- collecting files', N)
    collect(path, '')

    zipfullpath = join(destination, zipname)
    with zipfile.ZipFile(zipfullpath, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for fullpath, destpath in files:
            zipf.write(fullpath, destpath)


def copy_and_modify_html(gamedir, zipname, destination, mainfile=None):
    mainfile = mainfile or 'main.lua'
    htmlfile = locate_recursively(os.path.abspath(gamedir), os.getcwd(),
                                  'index.html')
    if not htmlfile:
        print(E, 'cannot find index.html', N)
        sys.exit(1)
    print(G, '- copy', htmlfile, N)
    html = open(htmlfile, 'r').read()

    html = html.replace('{{{DRYSTAL_LOAD}}}', DRYSTAL_LOAD)
    html = html.replace('{{{DRYSTAL_ADD_ARGUMENTS}}}',
                        DRYSTAL_ADD_ARGUMENTS.replace('ARGS', str(['--zip=' + zipname, mainfile])))
    open(join(destination, 'index.html'), 'w').write(html)


def run_repack(args):
    zipname = 'game.zip'
    if not os.path.exists(args.destination):
        os.mkdir(args.destination)

    if os.path.isdir(args.PATH):
        directory = args.PATH
        file = None
    else:
        directory, file = os.path.split(args.PATH)

    cmake_update('build-web', EMSCRIPTEN_CMAKE_DEFINES)

    # copy html (and check which version of drystaljs is used
    copy_and_modify_html(directory, zipname, args.destination, mainfile=file)
    # copy drystaljs and its data
    prepare_drystaljs(args.destination)

    # pack game data and copy wgot files
    config = load_config(directory)
    package_data(directory, zipname, args.destination,
                 config, args.show_include)
    copy_wget_files(directory, config, args.destination, args.show_include)


def run_web(args):
    run_repack(args)
    from http.server import HTTPServer, SimpleHTTPRequestHandler
    addr, port = '127.0.0.1', 8000
    httpd = HTTPServer((addr, port), SimpleHTTPRequestHandler)
    for b in BROWSERS:
        if not execute([b, addr + ':' + str(port) + '/' + args.destination]):
            print(G, '- page opened in', b, N)
            break
        else:
            print(W, '! unable to open a browser', N)
    httpd.serve_forever()


def get_gdb_args(program, pid=None, arguments=None):
    # if debug, run with 'gdb' (and give it path to sources)
    source_directories = ['src', os.path.join('external', 'lua', 'src')]
    args = ['gdb']
    for d in source_directories:
        args.append('-d')
        args.append(os.path.join('..', d))
    if pid:
        args += [program, str(pid), '-ex', 'handle SIGUSR1 ignore']
    else:
        args += ['-ex', 'run', '--args', program] + arguments
    return args


def run_native(args):
    wd, filename = os.path.split(args.PATH)
    program, arguments = prepare_native(args.release, False)

    if filename:  # other main file
        arguments.append(filename)
    if args.live:
        arguments.append("--livecoding")

    if args.debug:
        execute(get_gdb_args(program, arguments=arguments), cwd=wd)
    elif args.profile:
        execute(['valgrind'] + VALGRIND_ARGS_PROFILE + [program] + arguments, cwd=wd)
    elif args.memcheck:
        execute(['valgrind'] + VALGRIND_ARGS_MEMCHECK + [program] + arguments, cwd=wd)
    else:
        execute([program] + arguments, cwd=wd)


def valid_path(path):
    if not os.path.exists(path):
        msg = "%r does not exist" % path
        raise argparse.ArgumentTypeError(msg)
    return path


if __name__ == '__main__':
    import argparse

    add_signal_handlers()

    parser = argparse.ArgumentParser(description='Drystal roadrunner !')
    subparsers = parser.add_subparsers(help='sub-commands')

    parser_native = subparsers.add_parser('native', help='run with drystal',
                                        description='run with drystal')
    parser_native.add_argument('PATH', help='<directory>[/filename.lua]',
                            type=valid_path)
    parser_native.set_defaults(func=run_native)
    parser_native.add_argument('-l', '--live', help='live coding (reload code \
                            when it has been modified)',
                            action='store_true', default=False)
    parser_native.add_argument('-r', '--release', help='compile in release mode',
                            action='store_true', default=False)
    group = parser_native.add_mutually_exclusive_group()
    group.add_argument('-d', '--debug', help='debug with gdb',
                    action='store_true', default=False)
    group.add_argument('-p', '--profile', help='profile with valgrind',
                    action='store_true', default=False)
    group.add_argument('-m', '--memcheck', help='check memory with valgrind',
                    action='store_true', default=False)

    parser_web = subparsers.add_parser('web', help='run in a browser',
                                    description='run in a browser')
    parser_web.add_argument('PATH', help='<directory>[/filename.lua]',
                            type=valid_path)
    parser_web.set_defaults(func=run_web)
    parser_web.add_argument('-i', '--show-include', help='show files that are (not) included',
                            action='store_true', default=False)
    parser_web.add_argument('-d', '--destination', help='folder where web files will be put',
                            default='web')

    parser_repack = subparsers.add_parser('repack', help='repack',
                                        description='repack')
    parser_repack.add_argument('PATH', help='<directory>[/filename.lua]',
                            type=valid_path)
    parser_repack.set_defaults(func=run_repack)
    parser_repack.add_argument('-i', '--show-include', help='show files that are (not) included',
                            action='store_true', default=False)
    parser_repack.add_argument('-d', '--destination', help='folder where web files will be put',
                            default='web')

    if len(sys.argv) > 1:
        args = parser.parse_args()
        args.func(args)
    else:
        parser.print_usage()
