# result is RE and input data print to stderr file
import sys

s = input()
while s is not None:
  a = s.split()
  print int(a[0]) + int(a[1])
  s = input()