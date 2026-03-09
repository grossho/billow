#!/usr/bin/env python

import timeit
print (timeit.timeit("a.extend(range(10*(2**24)))", "a=list()", number=1))
print (timeit.timeit("a.extend(range(10*(2**24)))", "import billow; a=billow.flist(linesize=4096)", number=1))
