#!/usr/bin/env python
# (c) Vladimir Yu. Stepanov 2009-2026, Russia, Saratov
# e-mail: vysster@gmail.com

import sys
import os

try:
    from distutils.core import setup, Extension
except ModuleNotFoundError:
    from setuptools import setup, Extension

version = "0.2.2.2"
name = "billow-%s" % version

cwd = os.getcwd()
include_dirs = [ ]
library_dirs = [ ]
libraries = [ ]
compile_scheme = ''
#compile_scheme = 'debug'
#compile_scheme = 'memdebug'
try:
    compile_scheme = open("COMPILE-SCHEME").readline().strip()
except IOError:
    pass
if compile_scheme == 'debug':
    cflags = [ '-Wall', '-fPIC', '-g', '-pg' ]
elif compile_scheme == 'memdebug':
    cflags = [ '-Wall', '-fPIC', '-g', '-pg', '-DMEMDEBUG' ]
else:
    cflags = [ '-Wall', '-O2' ]
    #cflags = [ '-Wall', '-fPIC', '-O2' ]
define_macros = [ ]

setup(
    name=name,
    version=version,
    author="Vladimir Yu. Stepanov",
    author_email="vysster@gmail.com",
    description="""
Module have several types: ring (double linked list), pair(light weight tuple), Nil-singletone

In billow SQL(NULL) for now is 'billow.Nil' type with some aggregate functions on cpython release, and 'expectone' as bridge aggregated function together with 'coalesce', 'count', 'min', 'max', 'minmax'. All group function get as tuple arguments - {name}'_at', and iteratable function - {name}'_it' for fun
python constructor - for free on memory limits, because it is the same C function.
""",
    url='not yet',
    py_modules=[],
    ext_modules=[
        Extension(
            name='billow',
            sources=[
                'pycompat.c',
                'pyexceptionlist.c',
                'bw_debug.c',
                'bw_nil.c',
                'bw_pair.c',
                'bw_ring.c',
                #'bw_flist.c',
                'bw_treedict.c',
                'pxgtc_balance_avl.c',
                'pxgtc_balance_rb.c',
                'pxgtc2qcl_next.c',
                'pxgtc2qcl_prev.c',
                'pxgtc_vnext.c',
                'pxgtc_vprev.c',
                'pxgtc_wnext.c',
                'pxgtc_wprev.c',
                'pxgtc_xnext.c',
                'pxgtc_xprev.c',
                'pxgtc_znext.c',
                'pxgtc_zprev.c',
                'pxgtc_selectmin.c',
                'pxgtc_selectmax.c',
                'pxgtc_pathmin.c',
                'pxgtc_pathmax.c',
                'billow.c',
            ],
            include_dirs=include_dirs,
            library_dirs=library_dirs,
            libraries=libraries,
            extra_compile_args=cflags,
            define_macros=define_macros
        ),
    ]
)
