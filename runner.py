#!/usr/bin/env python3
# coding: utf-8

import os
import sys
import time
import shutil
import signal
import fnmatch
import argparse
import subprocess
import configparser

G = '\033[92m'
I = '\033[95m'
W = '\033[93m'
E = '\033[91m'
N = '\033[m'

DRYSTAL_DATA = os.path.abspath('data')

BUILD_NATIVE_RELEASE = os.path.abspath('build-native-release')
BUILD_NATIVE_DEBUG = os.path.abspath('build-native-debug')
BUILD_WEB = os.path.abspath('build-web')

join = os.path.join

BINARY_DIRECTORY_NATIVE_RELEASE = join(BUILD_NATIVE_RELEASE, 'src')
BINARY_DIRECTORY_NATIVE_DEBUG = join(BUILD_NATIVE_DEBUG, 'src')
BINARY_DIRECTORY_NATIVE_WEB = join(BUILD_WEB, 'src')
EXTENSIONS_DIRECTORY = os.path.abspath('extensions')
EXTENSIONS_DIRECTORY_NATIVE_RELEASE = join(BUILD_NATIVE_RELEASE, 'extensions')
EXTENSIONS_DIRECTORY_NATIVE_DEBUG = join(BUILD_NATIVE_DEBUG, 'extensions')
EXTENSIONS_DIRECTORY_WEB = join(BUILD_WEB, 'extensions')

EMSCRIPTEN_ROOT_PATH = '/usr/lib/emscripten'
EMSCRIPTEN_CMAKE_DEFINES = ['CMAKE_TOOLCHAIN_FILE=../cmake/Emscripten.cmake',
                            'EMSCRIPTEN_ROOT_PATH=' + EMSCRIPTEN_ROOT_PATH,
                            'EMSCRIPTEN=1',
                            'CMAKE_BUILD_TYPE=Release']
PACKAGER = '/usr/lib/emscripten/tools/file_packager.py'
COMPRESSOR = '/usr/lib/emscripten/third_party/lzma.js/lzma-native'
DECOMPRESS_JS = '/usr/lib/emscripten/third_party/lzma.js/lzma-decoder.js'
DECOMPRESS_NAME = 'LZMA.decompress'

LIB_PATH_RELEASE = join(BUILD_NATIVE_RELEASE, 'external')
LIB_PATH_DEBUG = join(BUILD_NATIVE_DEBUG, 'external')
VALGRIND_ARGS_MEMCHECK = '--tool=memcheck'
VALGRIND_ARGS_PROFILE = '--tool=callgrind'

BROWSERS = 'chromium', 'firefox'

HAS_NINJA = subprocess.call(['which', 'ninja'], stdout=subprocess.DEVNULL) == 0
COMPRESS_DATA = True

DRYSTAL_LOAD_DATA = '<script async type="text/javascript" src="FILE"></script>'

DECOMPRESS_CODE = '''
<script type='text/javascript'>
var decompressWorker = new Worker('decompress.js');
var decompressCallbacks = [];
var decompressions = 0;
Module["decompress"] = function(data, callback) {
    var id = decompressCallbacks.length;
    decompressCallbacks.push(callback);
    decompressWorker.postMessage({ data: data, id: id });
    if (Module['setStatus']) {
        decompressions++;
        Module['setStatus']('Decompressing...');
    }
};
decompressWorker.onmessage = function(event) {
      decompressCallbacks[event.data.id](event.data.data);
      decompressCallbacks[event.data.id] = null;
      if (Module['setStatus']) {
              decompressions--;
              if (decompressions == 0) {
                        Module['setStatus']('');
                      }
            }
};
</script>
'''

# decompress will be used for data
DRYSTAL_LOAD = DECOMPRESS_CODE + \
    '<script async type="text/javascript" src="drystal.data.js"></script>' + \
    '<script type="text/javascript" src="drystal.js"></script>'

DRYSTAL_LOAD_COMPRESSED = '''
''' + DECOMPRESS_CODE + '''
<script async type="text/javascript" src="drystal.data.js"></script>
<script type='text/javascript'>
var compiledCodeXHR = new XMLHttpRequest();
compiledCodeXHR.open('GET', 'drystal.js.compress', true);
compiledCodeXHR.responseType = 'arraybuffer';
compiledCodeXHR.onload = function() {
    var arrayBuffer = compiledCodeXHR.response;
    if (!arrayBuffer) throw('Loading compressed code failed.');
    var byteArray = new Uint8Array(arrayBuffer);
    Module.decompress(byteArray, function(decompressed) {
        var source = Array.prototype.slice.apply(decompressed)
            .map(function(x) {
            return String.fromCharCode(x)
        }).join(''); // createObjectURL instead?
        var scriptTag = document.createElement('script');
        scriptTag.setAttribute('type', 'text/javascript');
        scriptTag.innerHTML = source;
        document.body.appendChild(scriptTag);
    });
};
compiledCodeXHR.send(null);
</script>
'''

DRYSTAL_ADD_ARGUMENTS = '''
<script type='text/javascript'>
Module[\'arguments\'] = ARGS;
</script>
'''


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
        strcmd += ' from ' + cwd

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
    if directory not in config:
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
            collect(path, join(directory, d))

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


def tup_update(build=''):
    if '.tup' not in os.listdir('.'):
        execute(['tup', 'init'])
    if execute(['tup', 'upd', build]) != 0:
        print(E, 'compilation failed, stopping.', N)
        sys.exit(1)


def cmake_update(build, definitions=[]):
    generator = HAS_NINJA and 'Ninja' or 'Unix Makefiles'
    compiler = HAS_NINJA and 'ninja' or 'make'
    if not os.path.exists(build):
        os.mkdir(build)
        defs = ['-D' + d for d in definitions]
        if execute(['cmake', '..', '-G', generator] + defs, cwd=build) != 0:
            print(E, 'cmake failed. Fix CMakeLists.txt and try again!', N)
            clean(build)
            sys.exit(1)
    if execute([compiler], cwd=build) != 0:
        print(E, compiler, 'failed, stopping.', N)
        sys.exit(1)


def prepare_native(release=False, filename=None):
    directory = ''
    build_type = ''
    lib_path = ''
    bin_path = ''
    extensions_directory = ''
    if release:
        directory = BUILD_NATIVE_RELEASE
        build_type = 'Release'
        lib_path = LIB_PATH_RELEASE
        bin_path = BINARY_DIRECTORY_NATIVE_RELEASE
        extensions_directory = EXTENSIONS_DIRECTORY_NATIVE_RELEASE
    else:
        directory = BUILD_NATIVE_DEBUG
        build_type = 'Debug'
        lib_path = LIB_PATH_DEBUG
        bin_path = BINARY_DIRECTORY_NATIVE_DEBUG
        extensions_directory = EXTENSIONS_DIRECTORY_NATIVE_DEBUG

    cmake_update(directory, ['CMAKE_BUILD_TYPE=' + build_type])
    os.environ['LD_LIBRARY_PATH'] = lib_path
    program = join(bin_path, 'drystal')
    arguments = ['--add-path=' + extensions_directory,
                 '--add-path=' + DRYSTAL_DATA]
    if filename:  # other main file
        arguments.append(filename)
    return program, arguments


def prepare_drystaljs(destination, use_compress_drystal):
    '''
        create web/decompress.js
        compress build-web/src/drystal.js to web/drystal.js.compressed
        or copy build-web/src/drystal.js to web/drystal.js
        create a package with drystal's .lua inside
    '''
    srcjs = join(BINARY_DIRECTORY_NATIVE_WEB, 'drystal.js')
    decompressjs = join(destination, 'decompress.js')
    jscompressed = join(destination, 'drystal.js.compress')
    js = join(destination, 'drystal.js')
    if not os.path.exists(decompressjs):
        print(G, '- create decompress.js', N)
        decompressor = open(decompressjs, 'w')
        decompressor.write(open(DECOMPRESS_JS).read())
        decompressor.write('''
                onmessage = function(event) {
                postMessage({ data: %s(event.data.data), id: event.data.id });
                };
                ''' % DECOMPRESS_NAME)
        decompressor.close()

    if has_been_modified(srcjs, js) and not use_compress_drystal:
        print(G, '- copy drystal.js', N)
        shutil.copyfile(srcjs, js)

    if has_been_modified(srcjs, jscompressed) and use_compress_drystal:
        print(G, '- compress drystal.js to drystal.js.compress', N)
        execute([COMPRESSOR], stdin=srcjs, stdout=jscompressed)

    drystaldata = join(destination, 'drystal.data')
    js_drystaldata_loader = join(destination, 'drystal.data.js')
    recreate = False
    for f in os.listdir(DRYSTAL_DATA):
        full = join(DRYSTAL_DATA, f)
        if has_been_modified(full, js_drystaldata_loader):
            recreate = True
            break
    if recreate:
        print(G, '- create drystal.data.js', N)
        extensions = [join(DRYSTAL_DATA, e)
                      for e in os.listdir(EXTENSIONS_DIRECTORY_WEB)
                      if e.endswith('.so')]
        for ext in extensions:
            open(ext, 'a').close()
        execute(['python2', PACKAGER, drystaldata, '--no-heap-copy',
                '--embed', DRYSTAL_DATA + '@/'],
                stdout=js_drystaldata_loader)
        for ext in extensions:
            os.remove(ext)


def package_data(path, compress, data_js, destination, config, verbose=False):
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
                    files.append(full + '@/' + dest)
                else:
                    if verbose:
                        print(W, '\t~ ', dest, N)

        # collect directories after files
        for d in dirs:
            collect(path, d)

    if verbose:
        print(G, '- collecting files', N)
    collect(path, '')

    fulldestjs = join(destination, data_js)
    fulldest = fulldestjs.replace('.js', '')
    compress_opt = compress and ['--compress', COMPRESSOR] or []
    execute(['python2', PACKAGER, fulldest, '--no-heap-copy',
             '--preload'] + files + compress_opt,
            stdout=fulldestjs)
    if compress:  # not sure why emscripten generate this
        os.remove(fulldest)


def copy_and_modify_html(gamedir, data_js, destination, mainfile=None):
    use_compress_drystal = False
    mainfile = mainfile or 'main.lua'
    htmlfile = locate_recursively(os.path.abspath(gamedir), os.getcwd(),
                                  'index.html')
    if not htmlfile:
        print(E, 'canno\'t find index.html', N)
        sys.exit(1)
    print(G, '- copy', htmlfile, N)
    html = open(htmlfile, 'r').read()
    html = html.replace('{{{DRYSTAL_LOAD_DATA}}}',
                        DRYSTAL_LOAD_DATA.replace('FILE', data_js))

    if '{{{DRYSTAL_LOAD}}}' in html:
        html = html.replace('{{{DRYSTAL_LOAD}}}', DRYSTAL_LOAD)
    else:
        html = html.replace('{{{DRYSTAL_LOAD_COMPRESSED}}}',
                            DRYSTAL_LOAD_COMPRESSED)
        use_compress_drystal = True
    html = html.replace('{{{DRYSTAL_ADD_ARGUMENTS}}}',
                        DRYSTAL_ADD_ARGUMENTS.replace('ARGS', str([mainfile])))
    open(join(destination, 'index.html'), 'w').write(html)
    return use_compress_drystal


def run_repack(args):
    data_js = 'game.data.js'
    if not os.path.exists(args.destination):
        os.mkdir(args.destination)

    if os.path.isdir(args.PATH):
        directory = args.PATH
        file = None
    else:
        directory, file = os.path.split(args.PATH)

    cmake_update('build-web', EMSCRIPTEN_CMAKE_DEFINES)

    # copy html (and check which version of drystaljs is used
    use_compress_drystal = copy_and_modify_html(directory, data_js,
                                                args.destination,
                                                mainfile=file)
    # copy drystaljs and its data
    prepare_drystaljs(args.destination, use_compress_drystal)

    # pack game data and copy wgot files
    config = load_config(directory)
    package_data(directory, not args.no_compress, data_js, args.destination,
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
        args += ['-ex', 'run', '--args', program] + arguments
    return args


def setup_live_coding(directory, file, drystal):
    print(G, '- settings up live coding', N)

    def has_modifications(directory, latest):
        for f in os.listdir(directory):
            if f.startswith('.'):
                continue
            full = os.path.join(directory, f)
            if os.path.isdir(full):
                if has_modifications(full, latest):
                    return True
            elif os.path.isfile(full):
                if os.path.getmtime(full) > latest:
                    return True
        return False

    now = time.time()
    try:
        while drystal.poll() is None:
            time.sleep(1)
            c = has_modifications(directory, now)
            if c:
                drystal.send_signal(signal.SIGUSR1)
                now = time.time()
    except KeyboardInterrupt:
        drystal.terminate()
        sys.exit(1)


def run_native(args):
    wd, filename = os.path.split(args.PATH)
    program, arguments = prepare_native(args.release, filename)
    if args.debug:
        if args.live:
            drystal = execute([program] + arguments, fork=True, cwd=wd)
            execute(get_gdb_args(program, pid=drystal.pid), fork=True)
        else:
            execute(get_gdb_args(program, arguments=arguments),
                    fork=False, cwd=wd)
    elif args.profile:
        drystal = execute(['valgrind', VALGRIND_ARGS_PROFILE, program] + arguments,
                          fork=args.live, cwd=wd)
    elif args.memcheck:
        drystal = execute(['valgrind', VALGRIND_ARGS_MEMCHECK, program] + arguments,
                          fork=args.live, cwd=wd)
    else:
        drystal = execute([program] + arguments, fork=args.live, cwd=wd)
    if args.live:
        setup_live_coding(wd, filename, drystal)


def valid_path(path):
    if not os.path.exists(path):
        msg = "%r does not exist" % path
        raise argparse.ArgumentTypeError(msg)
    return path


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
parser_web.add_argument('-n', '--no-compress', help='don\'t compress datas',
                        action='store_true', default=False)
parser_web.add_argument('-i', '--show-include', help='show files that are (not) included',
                        action='store_true', default=False)
parser_web.add_argument('-d', '--destination', help='folder where web files will be put',
                        default='web')

parser_repack = subparsers.add_parser('repack', help='repack',
                                      description='repack')
parser_repack.add_argument('PATH', help='<directory>[/filename.lua]',
                           type=valid_path)
parser_repack.set_defaults(func=run_repack)
parser_repack.add_argument('-n', '--no-compress', help='don\'t compress data',
                           action='store_true', default=False)
parser_repack.add_argument('-i', '--show-include', help='show files that are (not) included',
                        action='store_true', default=False)
parser_repack.add_argument('-d', '--destination', help='folder where web files will be put',
                        default='web')

if __name__ == '__main__':
    if len(sys.argv) > 1:
        args = parser.parse_args()
        args.func(args)
    else:
        parser.print_usage()
