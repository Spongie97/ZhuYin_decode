# ! /usr/bin/python3

import sys, re

dic = dict()

with open(sys.argv[1], 'r', encoding='big5hkscs') as infile:
	for line in infile:
		ZhuYin = re.split('[/\s]', line)
		word = ZhuYin[0]
	
		for begin in ZhuYin:
			if not begin:
				continue
			if begin[0] not in dic:
				dic[begin[0]] = set()
			dic[begin[0]].add(word)
infile.close()

with open(sys.argv[2], 'w', encoding='big5hkscs') as outfile:
	for key, value in dic.items():
		outfile.write(key + ' ' + ' '.join(str(x) for x in value) + '\n')
outfile.close()
