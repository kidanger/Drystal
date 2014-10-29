#!/usr/bin/env python3
# coding: utf-8
#
# This is a drystal developer tool, it is not intended to help game developpers.

from runner import *
import itertools
import random


prefix = 'BUILD_'
configs = {
'UTILS': [],
'GRAPHICS': [],
'EVENT': ['GRAPHICS'],
'STORAGE': ['UTILS'],
'AUDIO': [],
'FONT': ['GRAPHICS'],
'WEB': [],
'PARTICLE': ['GRAPHICS'],
'PHYSICS': [],
}
configs_names = ['UTILS', 'GRAPHICS', 'EVENT', 'STORAGE', 'AUDIO',
        'FONT', 'WEB', 'PARTICLE', 'PHYSICS']

def get_index_of(v):
    for i, c in enumerate(configs_names):
        if c == v:
            return i
    return -1


def get_boolean_permutations():
    permutations = set()
    permutations_tmp = list(itertools.product([True, False], repeat=len(configs)))
    for p in permutations_tmp:
        pl = list(p)
        for i, v in enumerate(p):
            if v:
                for dep in configs[configs_names[i]]:
                    pl[get_index_of(dep)] = True
        permutations.add(tuple(pl))

    return permutations


def get_defines_permutations():
    boolean_permutations = get_boolean_permutations()
    defines_permutations = []
    for p in boolean_permutations:
        defines = []
        for i, b in enumerate(p):
            if b:
                defines.append(prefix + configs_names[i] + '=YES')
            else:
                defines.append(prefix + configs_names[i] + '=NO')
        defines_permutations.append(defines)

    random.shuffle(defines_permutations)
    return defines_permutations


def run_web(args):
    defines_permutations = get_defines_permutations()
    for d in defines_permutations:
        cmake_update('build-web', EMSCRIPTEN_CMAKE_DEFINES + d, True)


def run_native(args):
    build_type = 'Debug'
    directory = BUILD_NATIVE_DEBUG
    if args.release:
        build_type = 'Release'
        directory = BUILD_NATIVE_RELEASE
    defines_permutations = get_defines_permutations()
    nperm = str(len(defines_permutations))
    print(I + 'Building '+ nperm + ' permutations...')
    for i, d in enumerate(defines_permutations):
        print(I + 'Permutation ' + str(i + 1) + '/' + nperm)
        cmake_update(directory, ['CMAKE_BUILD_TYPE=' + build_type] + NATIVE_CMAKE_DEFINES + d, True)


if __name__ == '__main__':
    import argparse

    add_signal_handlers()

    parser = argparse.ArgumentParser(description='Drystal: build\'em all! Try to build every permutations of compilation flags BUILD_***')
    subparsers = parser.add_subparsers(help='sub-commands')

    parser_native = subparsers.add_parser('native', help='native builds',
                                        description='native builds')
    parser_native.set_defaults(func=run_native)
    parser_native.add_argument('-r', '--release', help='compile in release mode',
                            action='store_true', default=False)

    parser_web = subparsers.add_parser('web', help='web builds',
                                    description='web builds')
    parser_web.set_defaults(func=run_web)
    parser_web.add_argument('-d', '--destination', help='folder where web files will be put', default='web')

    args = parser.parse_args()
    args.func(args)

