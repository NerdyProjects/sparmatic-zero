import sys
segmentMap = dict({'a' : 1, 'b' : 2, 'c' : 4, 'd' : 8, 'e' : 16, 'f' : 32, 'g1' : 64, 'g2' : 128, 'h' : 256, 'i' : 512, 'j' : 1024, 'k' : 2048, 'l' : 4096, 'm': 8192})
for line in sys.stdin:
	res = 0
	for c in line:
		if c in segmentMap:
			res += segmentMap[c]
			safe = 0
		elif c == 'g':
			safe = 'g'
		elif safe == 'g':
			if c == '1':
				res += segmentMap['g1']
			if c == '2':
				res += segmentMap['g2']
	print (res)
	print(hex(res))
