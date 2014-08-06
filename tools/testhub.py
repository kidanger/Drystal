#!/usr/bin/env python3
# coding: utf-8
#
# This is a drystal developer tool, it is not intended to help game developpers.

from runner import *
import time
import signal
import sys
import os
import threading

tests = [
'ttf/line.lua',
'ttf/position.lua',
'ttf/simple_draw.lua',
'audio/create.lua',
'audio/play.lua',
'audio/random/main.lua',
'storage/main.lua',
'misc/objects.lua',
'misc/engine_stop.lua',
'misc/compute_perf.lua',
'misc/unicode.lua',
'misc/livecode.lua',
'misc/pointer.lua',
'misc/title.lua',
'misc/relative.lua',
'misc/hooks.lua',
'misc/fps.lua',
'particle/simple.lua',
'particle/fire.lua',
'web/wget.lua',
'web/js.lua',
'physic/ball.lua',
'physic/crate/main.lua',
'physic/destroy.lua',
'network/client.lua',
'network/server.lua',
'graphics/draw_point.lua',
'graphics/precise.lua',
'graphics/colors.lua',
'graphics/resize/fixed_size.lua',
'graphics/resize/resizable.lua',
'graphics/buffer.lua',
'graphics/filter.lua',
'graphics/npot.lua',
'graphics/draw_polygon.lua',
'graphics/camera.lua',
'graphics/line.lua',
'graphics/shaders.lua',
'graphics/fullscreen.lua',
'graphics/perf.lua',
'graphics/sprites.lua',
'graphics/surface_manipulations.lua',
'graphics/fx.lua',
]

addr, port = '127.0.0.1', 8000

def http_server():
    from http.server import HTTPServer, SimpleHTTPRequestHandler
    httpd = HTTPServer((addr, port), SimpleHTTPRequestHandler)
    httpd.serve_forever()


def prepare_web(args):
    if not os.path.exists(args.destination):
        os.mkdir(args.destination)

    cmake_update('build-web', EMSCRIPTEN_CMAKE_DEFINES)
    prepare_drystaljs(args.destination)


def repack(test, destination):
    zipname = 'game.zip'
    directory, file = os.path.split(test)
    # copy html (and check which version of drystaljs is used
    copy_and_modify_html(directory, zipname, args.destination, mainfile=file)

    # pack game data and copy wgot files
    config = load_config(directory)
    package_data(directory, zipname, destination, config, False)
    copy_wget_files(directory, config, destination, False)


def run_web_test(args):
    for b in BROWSERS:
        if not execute([b, addr + ':' + str(port) + '/' + args.destination]):
            print(G, '- page opened in', b, N)
            break
        else:
            print(W, '! unable to open a browser', N)
            sys.exit(1)


def run_web(args):
    prepare_web(args)
    t = threading.Thread(target=http_server, daemon=True)
    t.start()
    for test in tests:
        if not args.test_type or test.startswith(args.test_type):
            wd, filename = os.path.split('tests/' + test)
            repack('tests/' + test, args.destination)
            if args.wait:
                run_web_test(args)
                time.sleep(args.wait)
            else:
                run_web_test(args)


def run_native(args):
    drystal, drystal_args = prepare_native(args.release, args.coverage)
    for test in tests:
        if not args.test_type or test.startswith(args.test_type):
            wd, filename = os.path.split('tests/' + test)
            test_args = list(drystal_args)
            test_args.append(filename)
            if args.wait:
                process = execute([drystal] + test_args, fork=True, cwd=wd)
                time.sleep(args.wait)
                process.send_signal(signal.SIGINT)
            else:
                execute([drystal] + test_args, cwd=wd)


if __name__ == '__main__':
    import argparse

    add_signal_handlers()

    parser = argparse.ArgumentParser(description='Drystal testhub!')
    parser.add_argument('-t', '--test-type', help='test type', choices=['ttf','graphics','network','misc','web','storage', 'physic', 'particle', 'audio'])
    parser.add_argument('-w', '--wait', type=int, help='tell the testhub to wait X seconds before running the next test. By default, you must quit the test.')
    subparsers = parser.add_subparsers(help='sub-commands')

    parser_native = subparsers.add_parser('native', help='run with drystal',
                                        description='run with drystal')
    parser_native.set_defaults(func=run_native)
    parser_native.add_argument('-r', '--release', help='compile in release mode',
                            action='store_true', default=False)
    parser_native.add_argument('-c', '--coverage', help='enable code coverage',
                            action='store_true', default=False)

    parser_web = subparsers.add_parser('web', help='run in a browser',
                                    description='run in a browser')
    parser_web.set_defaults(func=run_web)
    parser_web.add_argument('-d', '--destination', help='folder where web files will be put',
                            default='web')

    args = parser.parse_args()
    args.func(args)
