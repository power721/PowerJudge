#PowerJudge
Judge Core for PowerOJ on Linux platform.

Based on UESTC and HUST judge core.

## Description:
auto get data files from data directory.

judge_fast doesn't use ptrace and no RF result, use chroot and setuid for permission control. 


##Build:
    make

    make -e FAST_JUDGE=1 -e LOG_LEVEL=LOG_NOTICE


##Test:
    make test


##Install:
    make install


## Usage:
    /usr/local/bin/powerjudge -s 10000 -p 1000 -D ./data -d ./temp -t 1000 -m 65535 -l 2

* -s    solution id
* -p    problem id
* -t    time limit (default 1000 ms)
* -m    memory limit (default 65536 KB)
* -l    language id (C: 1, CPP: 2, Pascal: 3, Java: 4, Python: 5)
* -D    root data directory, e.g. ~/oj/data/
* -d    root work directory, e.g. ~/oj/temp/ (default ./)
