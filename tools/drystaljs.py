#!/usr/bin/env python3
# coding: utf-8

import os
import sys
import shutil
import fnmatch
import zipfile
import subprocess
import configparser
import urllib.request

BROWSERS = (
    'chromium --allow-file-access-from-files --user-data-dir=/tmp/drystal-browser',
    'firefox',
)

G = '\033[92m'
I = '\033[95m'
W = '\033[93m'
E = '\033[91m'
N = '\033[m'

DEFAULT_DRYSTAL_CFG = """
[options]
version      = prerelease
destination  = web
zip          = game.zip
html         = index.html
htmltemplate = index.html
arguments    = main.lua
publish      = yourbranch

[*]
*.lua
*.ttf
*.wav
*.png
*.ogg
wget=

"""

DEFAULT_INDEX_HTML = """
<!doctype html>
<html lang="en-us">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title>Emscripten-Generated Code</title>
    <style>
      html,body{margin:0;padding:0;width:100%;height:100%;overflow:hidden}
      .emscripten { padding-right: 0; margin-left: auto; margin-right: auto; display: block; }
      textarea.emscripten { font-family: monospace; width: 80%; }
      div.emscripten { text-align: center; }
      /* the canvas *must not* have any border or padding, or mouse coords will be wrong */
      canvas.emscripten { border: 0px none; }
    </style>
  </head>
  <body>
    <canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()"></canvas>
    <textarea class="emscripten" id="output" rows="8"></textarea>

    <script type='text/javascript'>
      // connect to canvas
      var Module = {
        print: (function() {
          var element = document.getElementById('output');
          element.value = ''; // clear browser cache
          return function(text) {
            text = Array.prototype.slice.call(arguments).join(' ');
            element.value += text + "\\n";
            element.scrollTop = 99999; // focus on bottom
          };
        })(),
        canvas: document.getElementById('canvas'),
      };
      Module.printErr = Module.print;
    </script>
    {{{DRYSTAL_ADD_ARGUMENTS}}}
    {{{DRYSTAL_LOAD}}}
  </body>
</html>
"""

DRYSTAL_LOAD = '<script type="text/javascript" src="drystal.js"></script>'
DRYSTAL_URL = 'https://github.com/kidanger/Drystal/releases/download/VERSION'

DRYSTAL_ADD_ARGUMENTS = '''
<script type='text/javascript'>
Module[\'arguments\'] = ARGS;
</script>
'''


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
    try:
        if fork:
            subprocess.Popen(args, cwd=cwd, stdin=stdin, stdout=stdout)
            return True
        else:
            return subprocess.call(args, cwd=cwd, stdin=stdin, stdout=stdout) == 0
    except OSError:
        return False


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


def parent(directory):
    return os.path.abspath(os.path.join(directory, os.pardir))


def locate_recursively(from_dir, to_dir, name):
    directory = from_dir
    while directory != '/':
        files = os.listdir(directory)
        if name in files:
            return os.path.join(directory, name)
        if directory == to_dir:
            break
        directory = parent(directory)


def load_config(from_directory):
    config = configparser.ConfigParser(allow_no_value=True)
    config.optionxform = str  # upper case is important too
    cfg = locate_recursively(os.path.abspath(from_directory),
                             os.getcwd(), 'drystal.cfg')
    if not cfg:
        print(E + 'cannot find drystal.cfg', N)
        sys.exit(1)
    print(G, '- reading configuration from', cfg, N)
    config.read(cfg)
    return config, config_get_options(config)


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


def config_get_options(config):
    options = {}

    if 'options' not in config:
        print(E + '\'options\' section not found in configuration', N)
        sys.exit(1)

    def fill(opt, default=None):
        if opt in config['options']:
            options[opt] = config['options'][opt]
        elif default:
            options[opt] = default
        else:
            print(E + 'option \'' + opt + '\' not found in configuration', N)
            sys.exit(1)

    fill('version')
    fill('destination', 'web')
    fill('zip', 'game.zip')
    fill('html', 'index.html')
    fill('htmltemplate', 'index.html')
    fill('arguments', 'main.lua')
    fill('publish', 'unknown')
    return options

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


def package_data(path, config, options, verbose=False):
    files = []

    def collect(path, directory):
        dirs = []
        for f in os.listdir(os.path.join(path, directory)):
            if f.startswith('.'):
                continue
            dest = os.path.join(directory, f)
            full = os.path.join(path, dest)
            if os.path.isdir(full) and config_include_directory(config, dest):
                dirs.append(dest)
                files.append((full, dest))
                if verbose:
                    print(G, '\t+ ', dest + '/', N)
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

    zipfullpath = os.path.join(options['destination'], options['zip'])
    with zipfile.ZipFile(zipfullpath, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for fullpath, destpath in files:
            zipf.write(fullpath, destpath)


def copy_wget_files(path, config, options, verbose=False):
    print(G, '- copying wgot files', N)
    files = []

    def collect(path, directory):
        dirs = []
        for f in os.listdir(os.path.join(path, directory)):
            if f.startswith('.'):
                continue
            dest = os.path.join(directory, f)
            full = os.path.join(path, dest)
            if os.path.isdir(full) and config_include_directory(config, dest):
                dirs.append(dest)
            elif os.path.isfile(full):
                if config_is_wgetted(config, directory, f):
                    files.append(dest)
        for d in dirs:
            collect(path, d)

    collect(path, '')

    for f in files:
        dir = os.path.join(options['destination'], os.path.split(f)[0])
        if not os.path.exists(dir):
            os.makedirs(dir)

        if verbose:
            print(G, '\t@ ', f, N)
        src = os.path.join(path, f)
        dst = os.path.join(options['destination'], f)
        if has_been_modified(src, dst):
            shutil.copy(src, dst)


def wget(url, dest):
    print(G, '- wget', dest, N)
    try:
        response = urllib.request.urlopen(url)
        open(dest, 'wb').write(response.read())
        return True
    except urllib.error.HTTPError:
        return False


def generate_html(gamedir, options):
    htmlfile = locate_recursively(os.path.abspath(gamedir), os.getcwd(), options['htmltemplate'])

    if not htmlfile:
        print(E + 'cannot find', options['htmltemplate'], N)
        sys.exit(1)

    html = open(htmlfile, 'r').read()

    html = html.replace('{{{DRYSTAL_LOAD}}}', DRYSTAL_LOAD)
    argumentsstr = DRYSTAL_ADD_ARGUMENTS.replace('ARGS',
            str(['--zip=' + options['zip'], options['arguments']]))
    html = html.replace('{{{DRYSTAL_ADD_ARGUMENTS}}}', argumentsstr)
    print(G, '- generate', options['html'], N)
    open(os.path.join(options['destination'], options['html']), 'w').write(html)


def download_drystal(options):
    url = DRYSTAL_URL.replace('VERSION', options['version'])
    destjs = os.path.join(options['destination'], 'drystal.js')
    if not os.path.exists(destjs) and not wget(url + '/drystal.js', destjs):
        print(E + 'cannot download drystal.js, are you sure the version \''
                + options['version'] + '\' exists?', N)
        sys.exit(1)
    if not os.path.exists(destjs + '.mem') and not wget(url + '/drystal.js.mem', destjs + '.mem'):
        print(E + 'cannot download drystal.js.mem, are you sure the version \''
                + options['version'] + '\' exists?', N)
        sys.exit(1)

def copy_drystal(options, directory):
    destination = options['destination']
    srcjs = os.path.join(directory, 'drystal.js')
    srcjsmem = os.path.join(directory, 'drystal.js.mem')
    js = os.path.join(destination, 'drystal.js')
    jsmem = os.path.join(destination, 'drystal.js.mem')

    if has_been_modified(srcjs, js):
        print(G, '- copy drystal.js', N)
        shutil.copyfile(srcjs, js)
    if os.path.exists(srcjsmem) and has_been_modified(srcjsmem, jsmem):
        print(G, '- copy drystal.js.mem', N)
        shutil.copyfile(srcjsmem, jsmem)


def try_launch_browser(options):
    for b in BROWSERS:
        cmd = b.split(' ') + [os.path.join(os.getcwd(), 'web', 'index.html')]
        if execute(cmd):
            print(G, '- page opened in', cmd[0], N)
            break
    else:
        print(W, '! unable to open a browser', N)


def init(args):
    if not os.path.exists('drystal.cfg'):
        print(G, '- create drystal.cfg', N)
        open('drystal.cfg', 'w').write(DEFAULT_DRYSTAL_CFG)

    directory = os.getcwd()
    config, options = load_config(directory)

    if not os.path.exists(options['htmltemplate']):
        print(G, '- create', options['htmltemplate'], N)
        open(options['htmltemplate'], 'w').write(DEFAULT_INDEX_HTML)


def clean(args):
    directory = os.getcwd()
    config, options = load_config(directory)
    destination = options['destination']
    if os.path.exists(destination):
        print(G, '- remove', destination, N)
        shutil.rmtree(destination)


def pack(args):
    directory = os.getcwd()
    config, options = load_config(directory)

    if not os.path.exists(options['destination']):
        os.mkdir(options['destination'])

    package_data(directory, config, options, True)
    copy_wget_files(directory, config, options, True)
    generate_html(directory, options)
    if args.local:
        copy_drystal(options, args.local)
    else:
        download_drystal(options)


def run(args):
    directory = os.getcwd()
    config, options = load_config(directory)
    try_launch_browser(options)


def publish(args):
    directory = os.getcwd()
    config, options = load_config(directory)
    destination = options['destination']
    if not os.path.exists(destination):
        print(E + 'game not packed, please run \'drystaljs pack\'')

    dostash = not execute(['git', 'diff', '--quiet']) or not execute(['git', 'diff', '--cached', '--quiet'])
    if dostash:
        print(G, '- stashing', N)
        execute(['git', 'stash', '--quiet'])
    execute(['git', 'checkout', options['publish']])
    files = []
    for f in os.listdir(destination):
        full = os.path.join(destination, f)
        try:
            if os.path.isdir(full):
                shutil.copytree(full, f)
            else:
                shutil.copy(full, f)
            print(I, '- copy', f, N)
        except OSError:
            print(W, '- don\'t copy', f + ', already there', N)
        files.append(f)
    execute(['git', 'add'] + files)
    docommit = not execute(['git', 'diff', '--quiet']) or not execute(['git', 'diff', '--cached', '--quiet'])
    if docommit:
        print(G, '- commiting', N)
        execute(['git', 'commit'])
        if not args.no_push:
            print(G, '- pushing', N)
            execute(['git', 'push'])
    else:
        print(G, '- nothing to commit', N)
    execute(['git', 'checkout', 'master'])
    if dostash:
        print(G, '- poping stash', N)
        execute(['git', 'stash', 'pop', '--quiet'])


if __name__ == '__main__':
    def valid_directory(path):
        if not os.path.exists(path):
            msg = "%r does not exist" % path
            raise argparse.ArgumentTypeError(msg)
        if not os.path.isdir(path):
            msg = "%r is not a directory" % path
            raise argparse.ArgumentTypeError(msg)
        files = os.listdir(path)
        if 'drystal.js' not in files or 'drystal.js.mem' not in files:
            msg = "drystal.js or drystal.js.mem not found in %r" % path
            raise argparse.ArgumentTypeError(msg)
        return path

    import argparse

    parser = argparse.ArgumentParser(description='Drystal Javascript Utility')
    subparsers = parser.add_subparsers(help='sub-commands')

    parser_init = subparsers.add_parser('init', help='initialize configuration',
                                        description='initialize configuration')
    parser_init.set_defaults(func=init)

    parser_clean = subparsers.add_parser('clean', help='clean directory',
                                        description='clean directory')
    parser_clean.set_defaults(func=clean)

    parser_run = subparsers.add_parser('run', help='run in a browser',
                                    description='run in a browser')
    parser_run.set_defaults(func=run)

    parser_pack = subparsers.add_parser('pack', help='pack',
                                        description='pack')
    parser_pack.add_argument('-l', '--local', help='use a local javascript file',
                            type=valid_directory, metavar='PATH')
    parser_pack.set_defaults(func=pack)

    parser_publish = subparsers.add_parser('publish', help='publish',
                                        description='publish')
    parser_publish.add_argument('-n', '--no-push', help='don\'t push after commiting',
                            action='store_true', default=False)
    parser_publish.set_defaults(func=publish)

    if len(sys.argv) > 1:
        args = parser.parse_args()
        args.func(args)
    else:
        parser.print_usage()

