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

def prepare_002_tests():
    global refs
    refs = [ sys.getrefcount(i) for i in elements ]

def test_002_check_permutations():
    for i in itertools.permutations(elements, len(elements)):
        a = billow.pair(billow.min_it(i), billow.max_it(i))
        b = billow.minmax_it(i)
        c = billow.pair(min(i), max(i))
        assert a == c, (a, c)
        assert b == c, (b, c)

def test_002_check_permutations2():
    elements_some = [
        [],
        [ billow.Nil ],
        [ billow.Nil, billow.Nil ],
        [ billow.Nil, billow.Nil, billow.Nil ],
        [ billow.Nil, billow.Nil, billow.Nil, billow.Nil ],
        [ billow.Nil, elements[0] ],
        [ billow.Nil, billow.Nil, elements[0] ],
        [ billow.Nil, billow.Nil, billow.Nil, elements[0] ],
        [ billow.Nil, billow.Nil, billow.Nil, billow.Nil, elements[0] ],
        [ billow.Nil, elements[0], elements[1] ],
        [ billow.Nil, billow.Nil, elements[0], elements[1] ],
        [ billow.Nil, billow.Nil, billow.Nil, elements[0], elements[1] ],
        [ billow.Nil, elements[0], elements[1], elements[2] ],
        [ billow.Nil, billow.Nil, elements[0], elements[1], elements[2] ],
        [ billow.Nil, billow.Nil, billow.Nil, elements[0], elements[1], elements[2] ],
    ]
    for elements_check in elements_some:
        for i in itertools.permutations(elements_check, len(elements_check)):
            #print("i='", i, "'")
            a = (billow.min_it(i), billow.max_it(i))
            b = tuple(billow.minmax_it(i))
            assert a == b, (a, b)

def test_002_check_refs():
    refs_test = [ sys.getrefcount(i) for i in elements ]
    assert refs == refs_test, (refs, refs_test)
    assert refs_nil == sys.getrefcount(billow.Nil)

if __name__ == '__main__':
    prepare_002_tests()
    test_002_check_permutations()
    test_002_check_permutations2()
    test_002_check_refs()
