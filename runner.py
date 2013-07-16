#!/usr/bin/env python3
# coding: utf-8

import sys
import os
import shutil

DESTINATION_DIRECTORY = os.path.abspath('data')
DESTINATION_DIRECTORY_TMP = os.path.abspath('.data')

def parent(dir):
    return os.path.abspath(os.path.join(dir, os.pardir))

def copy_files_maybe(from_directory, to_directory):
    print('copy from', from_directory, 'to', to_directory)
    already_in = os.listdir(to_directory)
    for f in os.listdir(from_directory):
        fullpath = os.path.join(from_directory, f)
        if os.path.isfile(fullpath):
            if f not in already_in:
                print('copying', f)
                shutil.copy(fullpath, to_directory)
            else:
                print('already have', f)

def clean():
    if not os.path.exists(DESTINATION_DIRECTORY_TMP):
        return False
    shutil.rmtree(DESTINATION_DIRECTORY)
    shutil.copytree(DESTINATION_DIRECTORY_TMP, DESTINATION_DIRECTORY)
    shutil.rmtree(DESTINATION_DIRECTORY_TMP)
    return True

cw = os.getcwd()


if len(sys.argv) != 2 or not (os.path.exists(sys.argv[1]) or sys.argv[1] == 'clean'):
    print('usage:', sys.argv[0], '<directory>[/filename.lua]')
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

    to_be_run = sys.argv[1]

    if os.path.isdir(to_be_run):
        # in case the trailing '/' is omitted, because path.split wouldn't behave as we want
        dirpath = to_be_run
        file = None
    else:
        dirpath, file = os.path.split(to_be_run)

    # here, if file is not null, it will be renamed to 'main.lua'
    # if it's null, we asume there's already a 'main.lua' in dirpath

    dir = os.path.abspath(dirpath)

    shutil.copytree(DESTINATION_DIRECTORY, DESTINATION_DIRECTORY_TMP)
    while len(cw) < len(dir):
        copy_files_maybe(dir, DESTINATION_DIRECTORY)
        dir = parent(dir)

    if file and file != 'main.lua':
        print('rename', file, 'to main.lua')
        main = os.path.join(DESTINATION_DIRECTORY, 'main.lua')
        if os.path.exists(main):
            os.remove(main)
        os.rename(os.path.join(DESTINATION_DIRECTORY, file), main)

