#!/usr/local/bin/python
# -*- coding: koi8-r -*-

import billow
import sys
import itertools

upperlimit = 6
nillimit = 3
dupvalue = 2
def refcounts():
    print("nil refcount =", sys.getrefcount(billow.Nil))

try:
    xrange
except NameError:
    xrange = range

def test_001_check_nil_into_builtin_module():
    localvar = Nil
    assert localvar is billow.Nil

def test_001_check_empty_args():
    assert billow.min_it(()) is billow.Nil
    assert billow.max_it(()) is billow.Nil
    p = billow.minmax_it(())
    assert p[0] is billow.Nil and p[1] is billow.Nil

def test_001_check_nils():
    for i in xrange(1, upperlimit):
        l = [ billow.Nil ]*i
        assert billow.min_it(l) is billow.Nil, l
        assert billow.max_it(l) is billow.Nil, l
        p = billow.minmax_it(l)
        assert p[0] is billow.Nil and p[1] is billow.Nil, l

def test_001_min_permutations():
    for i in xrange(1, upperlimit):
        for j in xrange(nillimit):
            line = list(xrange(i)) + list([ billow.Nil ]*j)
            line[0] = dupvalue # duplicate one value
            for l in itertools.permutations(line):
                rmin = billow.min_it(l)
                if i > 1:
                    assert rmin == 1, (rmin, l)
                else:
                    assert rmin == dupvalue, (rmin, l)

def test_001_max_permutations():
    for i in xrange(1, upperlimit):
        for j in xrange(nillimit):
            line = list(xrange(i)) + list([ billow.Nil ]*j)
            line[0] = dupvalue # duplicate one value
            for l in itertools.permutations(line):
                rmax = billow.max_it(l)
                assert rmax == max(i - 1, dupvalue), (rmax, l)

def test_001_minmax_permutations():
    for i in xrange(1, upperlimit):
        for j in xrange(nillimit):
            line = list(xrange(i)) + list([ billow.Nil ]*j)
            line[0] = dupvalue # duplicate one value
            for l in itertools.permutations(line):
                rmin, rmax = billow.minmax_it(l)
                if i > 1:
                    assert rmin == 1, (rmin, l)
                else:
                    assert rmin == dupvalue, (rmin, l)
                assert rmax == max(i - 1, dupvalue), (rmax, l)

if __name__ == '__main__':
    refcounts()
    test_001_check_nil_into_builtin_module()
    test_001_check_empty_args()
    test_001_check_nils()
    test_001_min_permutations()
    test_001_max_permutations()
    test_001_minmax_permutations()
    refcounts()
