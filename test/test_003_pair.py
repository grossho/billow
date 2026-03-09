#!/usr/local/bin/python

import itertools
import billow
import sys

try:
    xrange
except NameError:
    xrange = range

elements = list(xrange(1000, 1006))
refs_nil = sys.getrefcount(billow.Nil)
refs = None

def prepare_003_tests():
    p = billow.pair(1,2)
    assert tuple(p[-1:2:1]) == (1,2)
    assert tuple(p[-10002:2:7]) == (2,)
    assert tuple(p[-1:2:2]) == (2,)
    assert tuple(p[2:2:2]) == ()
    assert tuple(p[2:0:-1]) == (2,)
    assert tuple(p[3:-1:-1]) == (2,1)
    assert tuple(p[8:-1:-1]) == (2,1)
    assert tuple(p[8:-1:-2]) == (1,)


if __name__ == '__main__':
    prepare_003_tests()
