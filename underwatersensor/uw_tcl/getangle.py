#-*- coding=utf8 -*-
__author__ = 'diaoboyu'

'''
get angle plot
'''

import matplotlib.pyplot as plt
import numpy as np

f = open('rlt')

lines = f.readlines()

anglelist = [[],[],[],[]]

i = 0
print len(lines)
total = 0

for s in lines:
	if s.startswith('angle'):
		total += 1
		anglelist[i].append(float(s.split(' ')[-1]))
		i += 1
		if i == 4:
			i = 0


print 'total=',total

x = range(0,500,10)
print x
#y = arange(0,90,5)
print anglelist[0]

print len(x),len(anglelist[1]),len(anglelist)

fig = plt.figure()

ax = fig.add_subplot(111)

plt.plot(x,anglelist[0])
plt.plot(x,anglelist[1])
plt.plot(x,anglelist[2])
plt.plot(x,anglelist[3])
#plt.plot(x,anglelist[1])

plt.show()
