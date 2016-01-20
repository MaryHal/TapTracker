#!/usr/bin/env python

import os
import sys
import fnmatch

import argparse
import itertools

import ninja_syntax

objdir = 'obj'

def flags(*args):
    return ' '.join(itertools.chain(*args))

def object_file(f):
    (root, ext) = os.path.splitext(f)
    return os.path.join(objdir, root + '.o')

def generate_files_from(directory, glob):
    for root, directories, files in os.walk(directory):
        for f in files:
            if fnmatch.fnmatch(f, glob):
                yield os.path.join(root, f)

def files_from(directory, glob):
    return list(generate_files_from(directory, glob))

# command line stuff
parser = argparse.ArgumentParser(usage='%(prog)s [options...]')
parser.add_argument('--debug', action='store_true', help='compile with debug flags')
parser.add_argument('--ci', action='store_true', help=argparse.SUPPRESS)
parser.add_argument('--cxx', metavar='<compiler>', help='compiler name to use (default: gcc)', default='gcc')
args = parser.parse_args()

BUILD_FILENAME = 'build.ninja'

compiler = args.cxx
include = ['-isystem./ext/uthash/include', '-isystem./ext/stb', '-isystem./ext/incbin', '-isystem./ext/flag']
depends = []
libdirs = []
ldflags = ['-lGL', '-lglfw', '-lm', '-lrt']
cxxflags = ['-Wall', '-Wextra', '-pedantic', '-std=c11']

if sys.platform == 'win32':
    project.libraries = ['mingw32']

def warning(string):
    print('warning: {}'.format(string))

# # configuration
# if 'g++' not in args.cxx:
#     warning('compiler not explicitly supported: {}'.format(args.cxx))

if args.debug:
    cxxflags.extend(['-g', '-O0', '-DDEBUG'])
else:
    cxxflags.extend(['-DNDEBUG', '-O3'])

if args.cxx == 'clang++':
    cxxflags.extend(['-Wno-constexpr-not-const', '-Wno-unused-value', '-Wno-mismatched-tags'])

### Build our ninja file
ninja = ninja_syntax.Writer(open('build.ninja', 'w'))

# Variables
ninja.variable('ninja_required_version', '1.3')
ninja.variable('ar', 'ar')
ninja.variable('cxx', compiler)
ninja.variable('cxxflags', flags(cxxflags + include + depends))
ninja.variable('ldflags', flags(libdirs + ldflags))
ninja.newline()

# Rules
ninja.rule('bootstrap', command = ' '.join(['python'] + sys.argv), generator = True)
ninja.rule('compile', command = '$cxx -MMD -MF $out.d -c $cxxflags $in -o $out',
deps = 'gcc', depfile = '$out.d',
description = 'Compiling $in to $out')
ninja.rule('link', command = '$cxx $cxxflags $in -o $out $ldflags', description = 'Creating $out')
ninja.rule('ar', command = 'rm -f $out && $ar crs $out $in', description = 'AR $out')
ninja.newline()

# Builds

ninja.build('build.ninja', 'bootstrap', implicit = 'bootstrap.py')

testobjs = []
srcfiles = files_from('src/', '**.c') + ['ext/flag/flag.c']
for f in srcfiles:
    obj = object_file(f)
    testobjs.append(obj)
    ninja.build(obj, 'compile', inputs = f)

ninja.newline()

ninja.build('./bin/TapTracker', 'link', inputs = testobjs)
