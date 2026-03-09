#!/usr/bin/env python

import billow
import timeit

iters = 50000
x = [2061, 2034, 2091, 2088, 2005, 2008, 2096, 2000, 2049, 2047, 2000.0, 2081, 2027, 2073, 2095, 2046, 2056, 2098, 2069, 2036, 2064, 2012, 2009, 2035, 2039, 2055, 2062, 2025, 2019, 2052, 2017, 2068, 2059, 2024, 2076, 2079, 2043, 2033, 2037, 2003, 2072, 2029, 2097, 2084, 2001, 2083, 2086, 2058, 2085, 2060, 2018, 2014, 2067, 2045, 2099, 2022, 2075, 2026, 2094, 2021, 2015, 2070, 2054, 2006, 2078, 2030, 2048, 2074, 2057, 2020, 2011, 2053, 2087, 2090, 2010, 2077, 2044, 2042, 2007, 2013, 2065, 2082, 2040, 2041, 2016, 2028, 2071, 2050, 2063, 2023, 2032, 2031, 2038, 2092, 2080, 2066, 2004, 2002, 2089, 2093, 2051, 2099.0]

print ("billow.minmax_it %s" % billow.minmax_it(x), billow.pair(min(x), max(x)))
print ((billow.min(x), billow.max(x)),billow.pair(min(x), max(x)))
print (tuple(billow.minmax_override_it(x)), (min(x), max(x)))
print ((billow.min_override(x), billow.max_override(x)),billow.pair(min(x), max(x)))
assert tuple(billow.minmax_it(x)) == (min(x), max(x))
assert tuple(billow.minmax_override_it(x)) == (min(x), max(x))
assert (billow.min_it(x), billow.max_it(x)) == (min(x), max(x))

r = repr(x)
t1 = timeit.Timer("minmax_it(x)", "x=%s\nfrom billow import minmax_it, min_it, max_it" % (r,)).timeit(number=iters)
t2 = timeit.Timer("(min_it(x), max_it(x))", "x=%s\nfrom billow import minmax_it, min_it, max_it" % (r,)).timeit(number=iters)
t3 = timeit.Timer("(min(x), max(x))", "x=%s\nfrom billow import minmax_it, min_it, max_it" % (r,)).timeit(number=iters)
t4 = timeit.Timer("(min(x))", "x=%s\nfrom billow import minmax_it, min_it, max_it" % (r,)).timeit(number=iters)
t5 = timeit.Timer("(max(x))", "x=%s\nfrom billow import minmax_it, min_it, max_it" % (r,)).timeit(number=iters)

print("'minmax_it(x)' time                  %.2f" % (t1,))
print("'(min_it(x), max_it(x))' time        %.2f" % (t2,))
print("'(min(x), max(x))' time              %.2f" % (t3,))
print("'(min(x))' time              %.2f" % (t4,))
print("'(max(x))' time              %.2f" % (t5,))
print()
print("'minmax_it(x)' is faster then '(min_it(x), max_it(x))' for %.2f%%" % (((t2 - t1)/t1)*100,))
print(" + 25% given by basic compare algorithm")
print(" + miscellaneous profit: two cycles vs. one cycle of generator loops")
print()
print("'minmax_it(x)' is faster then '(min(x), max(x))' for %.2f%%" % (((t3 - t1)/t1)*100,))
print(" + 25% given by basic compare algorithm")
print(" + miscellaneous profit: two cycles vs. one cycle of generator loops")
print(" * for short sequence (1-3 elements) profit is very low")
