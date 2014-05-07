#PowerJudge#
Judge Core for PowerOJ on Linux platform.

Based on UESTC and HUST judge core.

##Features##
auto get data files from data directory and sort by lexicographical.

set process limit for compiler, executor and spj.

chroot and setgid/setuid with nobody account for secuirty.

support Linux 32 bit and 64 bit.
  * Ubuntu 12.04 Desktop 64bit
  * Ubuntu 14.04 Server 32bit
  * Linux Mint 14 64bit
  * Debian 7.5 32bit
  * Debian 7.1 64bit

  * Redhat 5.5  **NOT** support
  * CentOS 4.8  **NOT** support

##Download##
    git clone http://git.oschina.net/power/PowerJudge.git


##Compilers##
    sudo apt-get install fpc openjdk-7-jdk python2.7 gcc g++

not support Eclipse Java Compiler or gcj-jdk.


##Build##
    make

or

    make -e LOG_LEVEL=LOG_NOTICE


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
* **-d**    root work directory, e.g. ~/oj/temp/
