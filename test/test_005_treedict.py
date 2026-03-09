#!/usr/local/bin/python

import billow
import timeit


def test_005_avl():
    td = billow.treedict(algono=0)
    assert td.setdefault(5,5) == 5
    assert td.setdefault(3,3) == 3
    assert td.setdefault(1,1) == 1
    assert td.setdefault(4,4) == 4
    assert td.setdefault(2,2) == 2
    assert td.setdefault(3,2) == 3
    assert td.popmin() == 1
    assert td.popmax() == 5
    assert td.pop(3) == 3
    assert td.get(2) == 2
    assert td.get(4) == 4
    assert td.get(1) is None
    assert td.get(3) is None
    assert td.get(5) is None
    print(td)

def test_005_rb():
    td = billow.treedict(algono=1)
    assert td.setdefault(5,5) == 5
    assert td.setdefault(3,3) == 3
    assert td.setdefault(1,1) == 1
    assert td.setdefault(4,4) == 4
    assert td.setdefault(2,2) == 2
    assert td.setdefault(3,2) == 3
    assert td.popmin() == 1
    assert td.popmax() == 5
    assert td.pop(3) == 3
    assert td.get(2) == 2
    assert td.get(4) == 4
    assert td.get(1) is None
    assert td.get(3) is None
    assert td.get(5) is None
    print(td)

def test_005_map():
    td = billow.treedict()
    a = 0
    try:
        td[1]
        a = 1
    except KeyError:
        pass
    assert a == 0
    assert td.setdefault(1,1) == 1
    assert td[1] == 1
    assert td.setdefault(2,3) == 3
    assert td.setdefault(3,4) == 4
    assert td.setdefault(4,5) == 5
    assert td.setdefault(5,6) == 6
    assert td[2] == 3
    td[2] = 4
    assert td[2] == 4

def test_005_clear_and_copy():
    td = billow.treedict()
    assert td.setdefault(1,1) == 1
    assert td.setdefault(2,3) == 3
    assert td.setdefault(3,4) == 4
    assert td.setdefault(4,5) == 5
    assert td.setdefault(5,6) == 6
    td2 = td.copy()
    td3 = td.xcopy()
    td.clear()
    print(td2)

def test_005_update():
    td = billow.treedict()
    td.update([('a', 'b'), billow.pair('b', 'c'), 'cd', ['d','e']])
    print(td)
    td.update({'e':'f', 'a': 'a'})
    print(td)

def test_005_fromkeys():
    td = billow.treedict.fromkeys('abc')
    print(td)
    td = billow.treedict.fromkeys(['a', 'b', 'c'], Nil)
    print(td)

def test_005_cmp_buffer():
    td = billow.treedict(compare=billow.cmp_buffer)
    td[b'abc'] = 1
    td[b'abd'] = 1
    td[b'abcd'] = 1
    td[b'abcde'] = 1
    td[b'abcdef'] = 1
    td[b'ab'] = 1
    print(td)

def test_005_cmp_long():
    try:
        td = billow.treedict(compare=billow.cmp_long)
        for i in range(100):
            td[i] = None
        print(td)
    except AttributeError:
        pass

def test_005_keysvaluesitems():
    td = billow.treedict()
    td.setdefault(1, 2)
    td.setdefault(2, 3)
    td.setdefault(0, 4)
    td.setdefault(1, 5)
    td.setdefault(4, 5)
    td.setdefault(5, 6)
    print(td)
    print(td, "keys =", td.keys(), "values =", td.values(), "items = ", td.items())
    print(td, "keys_inc =", td.keys_inc(), "values_inc =", td.values_inc(), "items_inc = ", td.items())
    print(td, "keys_dec =", td.keys_dec(), "values_dec=", td.values_dec(), "items_dec =", td.items_dec())


def test_005_timeit_copy_and_xcopy():
    t1 = timeit.Timer("x.xcopy()", "from billow import treedict\nx = treedict()\nfor i in range(1000000):\n    x[i] = None\n").timeit(number=1)
    t2 = timeit.Timer("x.copy()", "from billow import treedict\nx = treedict()\nfor i in range(1000000):\n    x[i] = None\n").timeit(number=1)
    t3 = timeit.Timer("x.copy()", "from billow import treedict\nx = dict()\nfor i in range(1000000):\n    x[i] = None\n").timeit(number=1)
    print("time treedict.xcopy   %8.3f" % (t1,))
    print("time treedict.copy    %8.3f" % (t2,))
    print("time dict             %8.3f" % (t3,))

def test_005_iters():
    a = billow.treedict()
    a.setdefault(3,1)
    a.setdefault(2,1)
    a.setdefault(1,1)
    a.setdefault(5,1)
    a.setdefault(4,1)
    assert a.keys() == list(a.iter_keys())
    assert a.values() == list(a.iter_values())
    assert a.items() == list(a.iter_items())
    assert a.keys_inc() == list(a.iter_keys_inc())
    assert a.values_inc() == list(a.iter_values_inc())
    assert a.items_inc() == list(a.iter_items_inc())
    assert a.keys_dec() == list(a.iter_keys_dec())
    assert a.values_dec() == list(a.iter_values_dec())
    assert a.items_dec() == list(a.iter_items_dec())
    assert list(a.iter_keys_inc_gt(2)) == [3,4,5]
    assert list(a.iter_keys_dec_lt(2)) == [1]




if __name__ == '__main__':
    test_005_avl()
    test_005_rb()
    test_005_map()
    test_005_clear_and_copy()
    test_005_update()
    test_005_fromkeys()
    test_005_cmp_buffer()
    test_005_cmp_long()
    test_005_keysvaluesitems()
    test_005_timeit_copy_and_xcopy()
    test_005_iters()
