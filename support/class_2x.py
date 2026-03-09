#!/usr/bin/env python2.7

import sys

d = {}
for i in dir(__builtins__):
	try:
		k = eval(i)
	except SyntaxError:
		sys.exc_clear()
		print "XXX", i
		continue
	if k.__class__ is type and issubclass(k, Exception):
		bases = map(lambda x: x.__name__, k.__bases__)
		if bases:
			if not d.has_key(bases[0]):
				d[bases[0]] = []
			d[bases[0]].append(i)

print d

for (parent, children) in d.items():
	print parent + ":"
	for k in children:
		if k != "" and k != None:
			print "\t" + k + ""
