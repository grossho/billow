#!/usr/bin/env python3.11

import types
import string



d = {}
for i in dir(__builtins__):
    k = eval(i)
    if k.__class__ is type and issubclass(eval(i), Exception):
        bases = list(map(lambda x: x.__name__, eval(i).__bases__))
        if bases:
            #print(bases)
            if bases[0] not in d.keys():
                d[bases[0]] = []
            d[bases[0]].append(i)

for (parent, children) in d.items():
    print(parent + ":")
    for k in children:
        if k != "" and k != None:
            print("\t" + k + "")
