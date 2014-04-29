import sys
 
for s in sys.stdin:
  a = [int(i) for i in s.split()]
  print a[0] + a[1]