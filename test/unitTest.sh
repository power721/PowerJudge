#!/bin/bash

start=`date +%s`

function init()
{
  passed=0
  failed=0
  mkdir -p temp/10000
  cp code/1000/1000_AC.cpp temp/10000/Main.cc
  mkdir temp/10001
  cp code/1000/1000_PE.cpp temp/10001/Main.cc
  mkdir temp/10002
  cp code/1000/1000_TLE.cpp temp/10002/Main.cc
  mkdir temp/10003
  cp code/1000/1000_MLE.cpp temp/10003/Main.cc
  mkdir temp/10004
  cp code/1000/1000_WA.cpp temp/10004/Main.cc
  mkdir temp/10005
  cp code/1000/1000_RE_SEGV.cpp temp/10005/Main.cc
  mkdir temp/10006
  cp code/1000/1000_OLE.cpp temp/10006/Main.cc
  mkdir temp/10007
  cp code/1000/1000_CE.cpp temp/10007/Main.cc
  mkdir temp/10008
  cp code/1000/1000_RF.cpp temp/10008/Main.cc
  
  cp ../java.policy temp/
  mkdir temp/10010
  cp code/1000/1000_AC.java temp/10010/Main.java
  mkdir temp/10011
  cp code/1000/1000_PE.java temp/10011/Main.java
  mkdir temp/10012
  cp code/1000/1000_TLE.java temp/10012/Main.java
  mkdir temp/10013
  cp code/1000/1000_MLE.java temp/10013/Main.java
  mkdir temp/10015
  cp code/1000/1000_RE.java temp/10015/Main.java
  mkdir temp/10017
  cp code/1000/1000_CE.java temp/10017/Main.java
  mkdir temp/20010
  cp code/1000/1000_thread.java temp/20010/Main.java

  mkdir temp/10025
  cp code/1000/1000_RE.py temp/10025/Main.py
  mkdir temp/10027
  cp code/1000/1000_CE.py temp/10027/Main.py

  mkdir temp/1003
  cp code/1000/1000.pas temp/1003/Main.pas
  
  mkdir temp/1004
  cp code/1000/1000.java temp/1004/Main.java
  
  mkdir temp/1005
  cp code/1000/1000.py temp/1005/Main.py

  mkdir temp/640
  cp code/1000/1000_lld.cpp temp/640/Main.cc
  mkdir temp/641
  cp code/1000/1000_I64d.cpp temp/641/Main.cc

  mkdir temp/14050
  cp code/1405/1405.cc temp/14050/Main.cc
  mkdir temp/14054
  cp code/1405/1405_WA.cc temp/14054/Main.cc

  mkdir temp/10009
  cp code/1000/1000_AC.cpp temp/10009/Main.cc
} 2> error.log

resultStr=(
  "AC"    \
  "PE"    \
  "TLE"   \
  "MLE"   \
  "WA"    \
  "RE"    \
  "OLE"   \
  "CE"    \
  "RF"    \
  "SE"    \
  "VE"    \
  "WAIT"  \
)

function runTest()
{
  local result=`$3`
  echo "Test $2:"
  read -a array <<< "$result"
  if [ -z "$array" ]; then
    array=(9 -1 -1)
  fi
  echo -e "Result: ${resultStr[$array]}  Time: ${array[1]} MS  Memory: ${array[2]} KB"

  if [ "$array" = "$1" ]; then
    echo -e "\x1b[32mPASS\x1b[0m"
    ((passed++))
  else
    echo -e "\x1b[31mFAIL\x1b[0m expect ${resultStr[$1]}"
    ((failed++))
  fi
} 2>> error.log

init

# normal test
runTest 0 "AC"  "../bin/powerjudge -s 10000 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 1 "PE"  "../bin/powerjudge -s 10001 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 2 "TLE" "../bin/powerjudge -s 10002 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 3 "MLE" "../bin/powerjudge -s 10003 -p 1000 -D ./data -d ./temp -t 1000 -m  1024 -l 2"
runTest 4 "WA"  "../bin/powerjudge -s 10004 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 5 "RE"  "../bin/powerjudge -s 10005 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 6 "OLE" "../bin/powerjudge -s 10006 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 7 "CE"  "../bin/powerjudge -s 10007 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 8 "RF"  "../bin/powerjudge -s 10008 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 9 "SE"  "../bin/powerjudge -s 10009 -p 1001 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 0 "Pascal AC" "../bin/powerjudge -s 1003 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 3"
runTest 0 "Java AC"   "../bin/powerjudge -s 1004 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 0 "Python AC" "../bin/powerjudge -s 1005 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 5"

# test Java
runTest 0 "Java AC"  "../bin/powerjudge -s 10010 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 1 "Java PE"  "../bin/powerjudge -s 10011 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 2 "Java TLE" "../bin/powerjudge -s 10012 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 3 "Java MLE" "../bin/powerjudge -s 10013 -p 1000 -D ./data -d ./temp -t 1000 -m 1024  -l 4"
runTest 5 "Java RE"  "../bin/powerjudge -s 10015 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 7 "Java CE"  "../bin/powerjudge -s 10017 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 0 "Java Thread"  "../bin/powerjudge -s 20010 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"

# test Python
runTest 5 "Python RE" "../bin/powerjudge -s 10025 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 5"
runTest 7 "Python CE" "../bin/powerjudge -s 10027 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 5"

# test int64
runTest 0 "long long" "../bin/powerjudge -s 640 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 0 "__int64"   "../bin/powerjudge -s 641 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"

# test spj
runTest 0 "SPJ AC" "../bin/powerjudge -s 14050 -p 1405 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 4 "SPJ WA" "../bin/powerjudge -s 14054 -p 1405 -D ./data -d ./temp -t 1000 -m 65535 -l 2"

echo -e "\nTotal Cases:" $((failed+passed))
echo -e "\x1b[31mFAILED\x1b[0m: $failed"
echo -e "\x1b[32mPASSED\x1b[0m: $passed"
end=`date +%s`
(( time=end-start ))
echo "Time:   ${time}s"