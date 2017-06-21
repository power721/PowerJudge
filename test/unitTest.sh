#!/bin/bash

function init()
{
  passed=0
  failed=0
  mkdir -p temp/100020
  cp code/1000/1000_AC.cc temp/100020/Main.cc
  mkdir temp/100021
  cp code/1000/1000_PE.cc temp/100021/Main.cc
  mkdir temp/100022
  cp code/1000/1000_TLE.cc temp/100022/Main.cc
  mkdir temp/100023
  cp code/1000/1000_MLE.cc temp/100023/Main.cc
  mkdir temp/100024
  cp code/1000/1000_WA.cc temp/100024/Main.cc
  mkdir temp/100025
  cp code/1000/1000_RE_SEGV.cc temp/100025/Main.cc
  mkdir temp/100026
  cp code/1000/1000_OLE.cc temp/100026/Main.cc
  mkdir temp/100027
  cp code/1000/1000_CE.cc temp/100027/Main.cc
  mkdir temp/100028
  cp code/1000/1000_RF.cc temp/100028/Main.cc
  mkdir temp/100129
  cp code/1001/1001_SE.cc temp/100129/Main.cc
  
  mkdir temp/1003
  cp code/1000/1000.pas temp/1003/Main.pas
  
  mkdir temp/1004
  cp code/1000/1000.java temp/1004/Main.java
  
  mkdir temp/1005
  cp code/1000/1000.py temp/1005/Main.py
  
  cp ../config/java.policy temp/
  mkdir temp/100040
  cp code/1000/1000_AC.java temp/100040/Main.java
  mkdir temp/100041
  cp code/1000/1000_PE.java temp/100041/Main.java
  mkdir temp/100042
  cp code/1000/1000_TLE.java temp/100042/Main.java
  mkdir temp/100043
  cp code/1000/1000_MLE.java temp/100043/Main.java
  mkdir temp/100045
  cp code/1000/1000_RE.java temp/100045/Main.java
  mkdir temp/100047
  cp code/1000/1000_CE.java temp/100047/Main.java
  mkdir temp/20040
  cp code/1000/1000_thread.java temp/20040/Main.java
  mkdir temp/20041
  cp code/1000/NotPublicClass.java temp/20041/Main.java

  mkdir temp/100053
  cp code/1000/1000_MLE.py temp/100053/Main.py
  mkdir temp/100055
  cp code/1000/1000_RE.py temp/100055/Main.py
  mkdir temp/100056
  cp code/1000/1000_OLE.py temp/100056/Main.py
  mkdir temp/100057
  cp code/1000/1000_CE.py temp/100057/Main.py

  mkdir temp/100060
  cp code/1000/1000.kt temp/100060/Main.kt

  mkdir temp/100010
  cp code/1000/1000.c temp/100010/Main.c
  
  mkdir temp/100017
  cp code/1000/1000_CE.c temp/100017/Main.c

  mkdir temp/100220
  cp code/1000/1000_AC.cc temp/100220/Main.cc

  mkdir temp/6420
  cp code/1000/1000_lld.cc temp/6420/Main.cc
  mkdir temp/6427
  cp code/1000/1000_I64d.cc temp/6427/Main.cc

  mkdir temp/140520
  cp code/1405/1405.cc temp/140520/Main.cc
  mkdir temp/140524
  cp code/1405/1405_WA.cc temp/140524/Main.cc

  mkdir temp/10
  cp code/1000/ac_no_header.c temp/10/Main.c
  mkdir temp/127
  cp code/1000/ce_no_header.cc temp/127/Main.cc

  cp ././../config/java.policy temp/
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

start=`date +%s`

# normal test
runTest 0 "AC"  "../bin/powerjudge -s 100020 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 1 "PE"  "../bin/powerjudge -s 100021 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 2 "TLE" "../bin/powerjudge -s 100022 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 3 "MLE" "../bin/powerjudge -s 100023 -p 1000 -D ./data -d ./temp -t 1000 -m  1024 -l 2"
runTest 4 "WA"  "../bin/powerjudge -s 100024 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 5 "RE"  "../bin/powerjudge -s 100025 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 6 "OLE" "../bin/powerjudge -s 100026 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 7 "CE"  "../bin/powerjudge -s 100027 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 8 "RF"  "../bin/powerjudge -s 100028 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 9 "SE"  "../bin/powerjudge -s 100129 -p 1001 -D /.data -d ./temp -t 1000 -m 65535 -l 2"
runTest 0 "GCC AC"    "../bin/powerjudge -s 100010 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 1"
runTest 7 "GCC CE"    "../bin/powerjudge -s 100017 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 1"
runTest 0 "Pascal AC" "../bin/powerjudge -s 1003 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 3"
runTest 0 "Java AC"   "../bin/powerjudge -s 1004 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 0 "Python AC" "../bin/powerjudge -s 1005 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 5"

# test Java
runTest 0 "Java AC"  "../bin/powerjudge -s 100040 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 1 "Java PE"  "../bin/powerjudge -s 100041 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 2 "Java TLE" "../bin/powerjudge -s 100042 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 3 "Java MLE" "../bin/powerjudge -s 100043 -p 1000 -D ./data -d ./temp -t 1000 -m 1024  -l 4"
runTest 5 "Java RE"  "../bin/powerjudge -s 100045 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 7 "Java CE"  "../bin/powerjudge -s 100047 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 0 "Java Thread"  "../bin/powerjudge -s 20040 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"
runTest 0 "Java No Public Class"  "../bin/powerjudge -s 20041 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 4"

# test Python
runTest 3 "Python MLE" "../bin/powerjudge -s 100053 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 5"
runTest 5 "Python RE"  "../bin/powerjudge -s 100055 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 5"
runTest 6 "Python OLE" "../bin/powerjudge -s 100056 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 5"
runTest 7 "Python CE"  "../bin/powerjudge -s 100057 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 5"

# test kotlin
runTest 0 "Kotlin AC"  "../bin/powerjudge -s 100060 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 6"

# test int64
runTest 0 "long long" "../bin/powerjudge -s 6420 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 7 "__int64"   "../bin/powerjudge -s 6427 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"

# test spj
runTest 0 "SPJ AC" "../bin/powerjudge -s 140520 -p 1405 -D ./data -d ./temp -t 1000 -m 65535 -l 2"
runTest 4 "SPJ WA" "../bin/powerjudge -s 140524 -p 1405 -D ./data -d ./temp -t 1000 -m 65535 -l 2"

# test empty data dir
runTest 0 "empty data dir"  "../bin/powerjudge -s 100220 -p 1002 -D ./data -d ./temp -t 1000 -m 65535 -l 2"

# gcc not include header for scanf/printf
runTest 0 "gcc no IO header" "../bin/powerjudge -s 10 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 1"

# g++ not include header for scanf/printf
runTest 7 "g++ no IO header" "../bin/powerjudge -s 127 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2"

echo -e "\nTotal Cases:" $((failed+passed))
echo -e "\x1b[31mFAILED\x1b[0m: $failed"
echo -e "\x1b[32mPASSED\x1b[0m: $passed"
end=`date +%s`
(( time=end-start ))
echo "Time:   ${time}s"
