#!/usr/local/bin/python

import billow

assert len(billow.ring()) == 0
assert len(billow.ring(range(100))) == 100
assert len(billow.ring()) == 0
assert len(billow.ring(range(20))) == 20

def test_004_ring_fromlast(hi=100, anchor=None):
    if anchor is None:
        r = billow.ring(range(hi))
    else:
        r = billow.ring(range(hi), anchor=anchor)
    lo = 0
    for lo in range(33):
        test = r.roll()
        assert test == lo, (test, lo, r)
    l1 = list(r)
    l2 = list(range(lo + 1, hi))

def test_004_ring_ptr():
    r = billow.ring([1,2,3])
    r.roll()
    r.roll()
    r.roll()
    try:
        r.roll()
    except StopIteration:
        pass
    else:
        raise ValueError("must be raised")
    assert list(r) == [], r


def test_004_ring_passthru():
    l6 = billow.ring(range(120))
    for i in range(21):
        l6.roll()
    for i in range(22):
        l6.roll()
    for i in range(23):
        l6.roll()
    for i in range(24):
        l6.roll()
    for i in range(25):
        l6.roll()
    assert list(l6) == list(range(115, 120)), l6

def test_004_ring_aggrwith():
    a = billow.ring(range(100), anchor='test')
    b = billow.ring(range(100, 120), anchor='test2')
    assert list(a.iter()) == list(range(100))
    assert list(b.iter()) == list(range(100, 120))
    a.aggrwith(b)
    assert list(a.iter()) == list(range(0, 120))

if __name__ == '__main__':
    test_004_ring_fromlast(100)
    test_004_ring_fromlast(133)
    test_004_ring_fromlast(67, 'test')
    test_004_ring_fromlast(300, 'test2')
    test_004_ring_passthru()
    test_004_ring_aggrwith()
