#!/usr/bin/env python3
# coding: utf-8
#
# This is a drystal developer tool, it is not intended to help game developpers.

import os
import sys
import shutil
import signal

from drystaljs import load_config, execute, has_been_modified, generate_html
from drystaljs import package_data, copy_wget_files, try_launch_browser, copy_drystal
from drystaljs import G, I, W, E, N

BUILD_NATIVE_RELEASE = os.path.abspath('build-native-release')
BUILD_NATIVE_DEBUG = os.path.abspath('build-native-debug')
BUILD_WEB_DEBUG = os.path.abspath('build-web-debug')
BUILD_WEB_RELEASE = os.path.abspath('build-web-release')

join = os.path.join

BINARY_DIRECTORY_NATIVE_RELEASE = join(BUILD_NATIVE_RELEASE, 'src')
BINARY_DIRECTORY_NATIVE_DEBUG = join(BUILD_NATIVE_DEBUG, 'src')
BINARY_DIRECTORY_WEB_DEBUG = join(BUILD_WEB_DEBUG, 'src')
BINARY_DIRECTORY_WEB_RELEASE = join(BUILD_WEB_RELEASE, 'src')

NATIVE_CMAKE_DEFINES = []

EMSCRIPTEN_ROOT = os.environ['EMSCRIPTEN'] if os.environ.get('EMSCRIPTEN') else ""
EMSCRIPTEN_CMAKE_DEFINES = ['CMAKE_TOOLCHAIN_FILE=' + EMSCRIPTEN_ROOT + '/cmake/Modules/Platform/Emscripten.cmake',
                            'BUILD_LIVECODING=NO',
                            ]

LIB_PATH_RELEASE = join(BUILD_NATIVE_RELEASE, 'external')
LIB_PATH_DEBUG = join(BUILD_NATIVE_DEBUG, 'external')
VALGRIND_ARGS_MEMCHECK = ['--tool=memcheck', '--leak-check=full', '--suppressions=' + os.path.abspath('./tools/drystal.supp')]
VALGRIND_ARGS_PROFILE = ['--tool=callgrind']

HAS_NINJA = shutil.which('ninja')


def signal_handler(signum, frame):
    print(I + 'Signal ' + str(signum) + ' caught, quitting...' + N)
    exit(0)

def add_signal_handlers():
    if os.name != 'nt':
        signal.signal(signal.SIGQUIT, signal_handler)
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)


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
        if not execute(['cmake', '..', '-G', generator] + defs, cwd=build):
            print(E + 'cmake failed. Fix CMakeLists.txt and try again!' + N)
            clean(build)
            sys.exit(1)
    if not execute([builder], cwd=build):
        print(E + builder, 'failed, stopping.' + N)
        sys.exit(1)


def run_target(build, target):
    builder = HAS_NINJA and 'ninja' or 'make'
    if not execute([builder, target], cwd=build):
        print(E + builder, target, 'failed, stopping.' + N)
        sys.exit(1)


def prepare_native(release=False, enable_coverage=False, disabled_modules=None):
    disabled_modules = disabled_modules or []
    directory = ''
    build_type = ''
    lib_path = ''
    bin_path = ''
    coverage = ''
    disabled_modules_prefixed = []
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

    for d in disabled_modules:
        disabled_modules_prefixed += ['BUILD_' + d.upper() + '=OFF']

    cmake_update(directory, ['CMAKE_BUILD_TYPE=' + build_type, 'BUILD_ENABLE_COVERAGE='+coverage] + NATIVE_CMAKE_DEFINES + disabled_modules_prefixed)
    if enable_coverage:
        run_target(directory, 'coverage-reset')

    os.environ['LD_LIBRARY_PATH'] = lib_path
    program = join(bin_path, 'drystal')
    return program, []


def prepare_drystaljs(options, release):
    directory = BINARY_DIRECTORY_WEB_DEBUG
    if release:
        directory = BINARY_DIRECTORY_WEB_RELEASE

    copy_drystal(options, directory)


def prepare_webbuild(release=False, disabled_modules=None):
    disabled_modules = disabled_modules or []
    directory = BUILD_WEB_DEBUG
    build_type = 'RelWithDebInfo'
    disabled_modules_prefixed = []

    if release:
        directory = BUILD_WEB_RELEASE
        build_type = 'MinSizeRel'

    for d in disabled_modules:
        disabled_modules_prefixed += ['BUILD_' + d.upper() + '=OFF']

    cmake_update(directory, ['CMAKE_BUILD_TYPE=' + build_type] + EMSCRIPTEN_CMAKE_DEFINES + disabled_modules_prefixed)


def run_repack(args):
    if os.path.isdir(args.PATH):
        directory = args.PATH
        file = None
    else:
        directory, file = os.path.split(args.PATH)

    config, options = load_config(directory)
    if file:
        options['arguments'] += file

    if not os.path.exists(options['destination']):
        os.mkdir(options['destination'])

    prepare_webbuild(args.release, args.disable_modules)

    prepare_drystaljs(options, args.release)
    generate_html(directory, options)

    package_data(directory, config, options, args.show_include)
    copy_wget_files(directory, config, options, args.show_include)


def run_web(args):
    if not EMSCRIPTEN_ROOT:
        print(E + 'Failed to build web version' + N)
        print(E + 'EMSCRIPTEN environment variable should contain the path to your emscripten installation' + N)
        sys.exit(1)

    run_repack(args)
    try_launch_browser(args)


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
    wd, filename = None, None
    if args.dir:
        wd = args.dir
        filename = args.PATH
    else:
        wd = args.PATH
        if os.path.isfile(args.PATH):
            wd, filename = os.path.split(args.PATH)
    program, arguments = prepare_native(args.release, False, args.disable_modules)

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


def run_unittest(args):
    args.PATH = 'tools/unittest.lua'
    args.disable_modules = []
    args.live = False
    args.dir = '.'
    run_native(args)


def valid_path(path):
    if not os.path.exists(path):
        msg = "%r does not exist" % path
        raise argparse.ArgumentTypeError(msg)
    return path


if __name__ == '__main__':
    import argparse

    add_signal_handlers()

    parser = argparse.ArgumentParser(description='Drystal roadrunner !')
    parser.add_argument('-r', '--release', help='compile in release mode',
                        action='store_true', default=False)
    subparsers = parser.add_subparsers(help='sub-commands')

    parser_native = subparsers.add_parser('native', help='run with drystal',
                                        description='run with drystal')
    parser_native.add_argument('PATH', help='<directory>[/main.lua]',
                            type=valid_path)
    parser_native.set_defaults(func=run_native)
    parser_native.add_argument('--dir', help='execute from directory',
                            type=valid_path)
    parser_native.add_argument('-l', '--live', help='live coding (reload code \
                            when it has been modified)',
                            action='store_true', default=False)
    parser_native.add_argument('-D', '--disable-modules', help='disable modules',
                        nargs='+', choices=['font', 'graphics', 'web', 'utils', 'storage', 'physics', 'particle', 'audio'])
    group = parser_native.add_mutually_exclusive_group()
    group.add_argument('-d', '--debug', help='debug with gdb',
                    action='store_true', default=False)
    group.add_argument('-p', '--profile', help='profile with valgrind',
                    action='store_true', default=False)
    group.add_argument('-m', '--memcheck', help='check memory with valgrind',
                    action='store_true', default=False)

    parser_web = subparsers.add_parser('web', help='run in a browser',
                                    description='run in a browser')
    parser_web.add_argument('PATH', help='<directory>[/main.lua]',
                            type=valid_path)
    parser_web.set_defaults(func=run_web)
    parser_web.add_argument('-i', '--show-include', help='show files that are (not) included',
                            action='store_true', default=False)
    parser_web.add_argument('-D', '--disable-modules', help='disable modules',
                        nargs='+', choices=['font', 'graphics', 'web', 'utils', 'storage', 'physics', 'particle', 'audio'])

    parser_repack = subparsers.add_parser('repack', help='repack',
                                        description='repack')
    parser_repack.add_argument('PATH', help='<directory>[/main.lua]',
                            type=valid_path)
    parser_repack.set_defaults(func=run_repack)
    parser_repack.add_argument('-i', '--show-include', help='show files that are (not) included',
                            action='store_true', default=False)
    parser_repack.add_argument('-D', '--disable-modules', help='disable modules',
                        nargs='+', choices=['font', 'graphics', 'web', 'utils', 'storage', 'physics', 'particle', 'audio'])

    parser_unittest = subparsers.add_parser('unittest', help='launch unit tests',
                                       description='launch unit tests')
    parser_unittest.set_defaults(func=run_unittest)
    group = parser_unittest.add_mutually_exclusive_group()
    group.add_argument('-d', '--debug', help='debug with gdb',
                    action='store_true', default=False)
    group.add_argument('-p', '--profile', help='profile with valgrind',
                    action='store_true', default=False)
    group.add_argument('-m', '--memcheck', help='check memory with valgrind',
                    action='store_true', default=False)

    if len(sys.argv) > 1:
        args = parser.parse_args()
        args.func(args)
    else:
        parser.print_usage()

