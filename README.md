#PowerJudge#
Judge Core for PowerOJ on Linux platform.

Based on UESTC and HUST judge core.

##Features##
auto get data files from data directory.

judge_fast doesn't use ptrace and no RF result, use chroot and setuid for permission control. 


##Build##
    make

or

    make -e FAST_JUDGE=1 -e LOG_LEVEL=LOG_NOTICE


##Test##
    make test


##Install##
    make install


##Usage##
    /usr/local/bin/powerjudge -s 10000 -p 1000 -t 1000 -m 65536 -l 2 -D ./data -d ./temp

* **-s**    solution id
* **-p**    problem id
* **-t**    time limit   (optional, default 1000 ms)
* **-m**    memory limit (optional, default 65536 KB)
* **-l**    language id
  1. C
  2. CPP
  3. Pascal
  4. Java
  5. Python
* **-D**    root data directory, e.g. ~/oj/data/
* **-d**    root work directory, e.g. ~/oj/temp/ (optional, default './')
