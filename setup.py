#!/usr/bin/env python

import sys
import os
from distutils.core import setup, Extension

version = "0.0.1"
name = "billow-%s" % version

cwd = os.getcwd()
include_dirs = [ ]
library_dirs = [ ]
libraries = [ ]
compile_scheme = ''
try:
    compile_scheme = open("COMPILE-SCHEME").readline().strip()
except IOError:
    sys.exc_clear()
if compile_scheme == 'debug':
    cflags = [ '-Wall', '-fPIC', '-g', '-pg' ]
elif compile_scheme == 'memdebug':
    cflags = [ '-Wall', '-fPIC', '-g', '-pg', '-DMEMDEBUG' ]
else:
    cflags = [ '-Wall', '-fPIC', '-O2' ]
define_macros = [ ]

setup(
	name=name,
	version=version,
	author="Vladimir Yu. Stepanov",
	author_email="grossho@gmail.com",
	description="billow module",
	url='not yet',
	py_modules=[],
	ext_modules=[
		Extension(
			name='billow',
			sources=[
				'billow.c',
				'billow_nil.c',
				'billow_pair.c',
                'pyexceptionlist.c',
			],
			include_dirs=include_dirs,
			library_dirs=library_dirs,
			libraries=libraries,
			extra_compile_args=cflags,
			define_macros=define_macros
		),
	]
)
