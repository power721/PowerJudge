/*
 * Copyright 2014 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
#ifndef SRC_JUDGE_CORE_H_
#define SRC_JUDGE_CORE_H_


#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>

// OJ结果代码
#define OJ_AC       0
#define OJ_PE       1
#define OJ_TLE      2
#define OJ_MLE      3
#define OJ_WA       4
#define OJ_RE       5
#define OJ_OLE      6
#define OJ_CE       7
#define OJ_RF       8
#define OJ_SE       9
#define OJ_VE       10
#define OJ_WAIT     11
#define OJ_RUN      12

char result_str[][10] = {
        "AC",
        "PE",
        "TLE",
        "MLE",
        "WA",
        "RE",
        "OLE",
        "CE",
        "RF",
        "SE",
        "VE",
        "WAIT",
        "Running"
};


// 常量
#define STD_KB 1024
#define STD_MB 1048576
#define PATH_SIZE 4096
#define BUFF_SIZE 8192
#define bool char
#define false 0
#define true  1
#define MAX_LOG_FILE_SIZE (STD_MB << 4)


// judge 类型
#define ACM 0
#define OI 1
#define TC 2
#define CF 3


// 退出原因
#define EXIT_OK                0
#define EXIT_UNPRIVILEGED      1
#define EXIT_CHDIR             2
#define EXIT_BAD_PARAM         3
#define EXIT_MISS_PARAM        4
#define EXIT_VERY_FIRST        5
#define EXIT_FORK_COMPILER     6
#define EXIT_COMPILE_IO        6
#define EXIT_COMPILE_EXEC      6
#define EXIT_COMPILE_ERROR     6
#define EXIT_NO_SOURCE_CODE    7
#define EXIT_PRIVILEGED        8
#define EXIT_PRE_JUDGE         9
#define EXIT_PRE_JUDGE_PTRACE  10
#define EXIT_PRE_JUDGE_EXECLP  11
#define EXIT_PRE_JUDGE_DAA     12
#define EXIT_FORK_ERROR        13
#define EXIT_EXEC_ERROR        14
#define EXIT_SET_LIMIT         15
#define EXIT_CURL_ERROR        16
#define EXIT_SET_SECURITY      17
#define EXIT_JUDGE             21
#define EXIT_COMPARE           27
#define EXIT_COMPARE_SPJ       30
#define EXIT_COMPARE_SPJ_FORK  31
#define EXIT_TIMEOUT           36
#define EXIT_UNKNOWN           127

#define GCC_COMPILE_ERROR      1

#define SPJ_AC                 0
#define SPJ_PE                 1
#define SPJ_WA                 2


// 程序语言
#define LANG_UNKNOWN           0
#define LANG_C11               1
#define LANG_CPP11             2
#define LANG_PASCAL            3
#define LANG_JAVA              4
#define LANG_PYTHON27          5
#define LANG_C99               6
#define LANG_CPP98             7
#define LANG_CPP14             8
#define LANG_CPP17             9
#define LANG_PYTHON3           10
const char languages[][10] = {"unknown", "gcc11", "g++11", "pascal", "java", "python2.7", "gcc99", "g++98", "g++14", "g++17", "python3"};
const char lang_ext[][10] = {"unknown", "c", "cc", "pas", "java", "py", "c", "cc", "cc", "cc", "py"};

#endif  // SRC_JUDGE_CORE_H_
