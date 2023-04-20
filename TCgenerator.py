import random

f = open("input_minterm.txt", 'w')
lst = []

f.write("1000\n")
for _ in range(2000) :
	num = random.randrange(0, 2**1000)
	if (num % 3 == 0) : s = "m "
	else : s = "d "
	s += str(format(num, 'b')).zfill(1000)
	if (s in lst) : continue
	lst.append(s)
 
	
for a in lst :
    f.write(a+'\n')

f.close()