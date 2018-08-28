//
// Created by w703710691d on 18-8-24.
//
/*
 * Copyright 2014 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
#ifndef POWERJUDGE_JUDGE_H
#define POWERJUDGE_JUDGE_H

#include <cstdio>
#include <cstdint>
#include "judge_core.h"

// 编译选项
const char *CP_C11[] = {"gcc", "Main.c", "-fno-asm", "-lm", "-static", "-Wall",
                        "-std=gnu11", "-O4", "-pipe", "-march=native", "-DONLINE_JUDGE", "-o", "Main", nullptr};
const char *CP_C99[] = {"gcc", "Main.c", "-fno-asm", "-lm", "-static", "-Wall",
                        "-std=gnu99", "-O4", "-pipe", "-march=native", "-DONLINE_JUDGE", "-o", "Main", nullptr};
const char *CP_CC11[] = {"g++", "Main.cc", "-fno-asm", "-lm", "-static", "-Wall",
                         "-std=gnu++11", "-O4", "-pipe", "-march=native", "-DONLINE_JUDGE", "-o", "Main", nullptr};
const char *CP_CC14[] = {"g++", "Main.cc", "-fno-asm", "-lm", "-static", "-Wall",
                         "-std=gnu++14", "-O4", "-pipe", "-march=native", "-DONLINE_JUDGE", "-o", "Main", nullptr};
const char *CP_CC17[] = {"g++", "Main.cc", "-fno-asm", "-lm", "-static", "-Wall",
                         "-std=gnu++17", "-O4", "-pipe", "-march=native", "-DONLINE_JUDGE", "-o", "Main", nullptr};
const char *CP_CC98[] = {"g++", "Main.cc", "-fno-asm", "-lm", "-static", "-Wall",
                         "-std=gnu++98", "-O4", "-pipe", "-march=native", "-DONLINE_JUDGE", "-o", "Main", nullptr};
const char *CP_PAS[] = {"fpc", "Main.pas", "-O2", "-Co", "-Cr", "-Ct", "-Ci", nullptr};
const char *CP_J[] = {"javac", "-g:none", "-Xlint", "-encoding", "UTF-8", "Main.java", nullptr};
const char *CP_PY27[] = {"python2.7", "-c", "import py_compile;py_compile.compile(r'Main.py')", nullptr};
const char *CP_PY3[] = {"python3", "-c", "import py_compile;py_compile.compile(r'Main.py')", nullptr};
const char *CP_KT[] = {"kotlinc", "Main.kt", nullptr};

// "-Xms512m", "-Xmx512m", "-Xss256k"
const char *EXEC_J[] = {"java", "-cp", ".", "-DONLINE_JUDGE=true", "-Djava.security.manager", "-Xss64m",
                        "-Djava.security.policy=/home/judge/java.policy", "Main", nullptr};
const char *EXEC_PY27[] = {"python2.7", "Main.py", nullptr};
const char *EXEC_PY3[] = {"python3", "Main.py", nullptr};
const char *EXEC_KT[] = {"kotlin", "-DONLINE_JUDGE=true", "MainKt", nullptr};

// configruation
// judge本身的时限(ms)
uint64_t judge_time_limit = 15347;

// 编译限制(ms)
uint64_t compile_time_limit = 5347;

// 编译限制(MB)
unsigned int compile_memory_limit = 1024;

// 编译输出限制(MB)
unsigned int compile_fsize_limit = 64;

// SPJ时间限制(ms)
unsigned int spj_time_limit = 10347;

// 程序运行的栈空间大小(KB)
unsigned int stack_size_limit = 65536;

// ms
unsigned int time_limit_addtion = 347;

unsigned int java_time_factor = 3;

unsigned int java_memory_factor = 3;

unsigned int python_time_factor = 3;

unsigned int python_memory_factor = 3;

unsigned int kotlin_time_factor = 3;

unsigned int kotlin_memory_factor = 3;

/* -- end of configruation -- */

off_t page_size;

char work_dir_root[PATH_SIZE] = ".";

char data_dir_root[PATH_SIZE];

struct oj_solution_t {
    int cid;           // contest id
    int sid;           // solution id
    int pid;           // problem id
    int lang;          // language id
    unsigned long time_limit;    // ms
    unsigned long memory_limit;  // KB

    int result;
    int judge_type;

    unsigned long time_usage;    // ms
    unsigned long memory_usage;  // KB

    bool spj;

    char work_dir[PATH_SIZE];
    char data_dir[PATH_SIZE];
    char spj_exe_file[PATH_SIZE];
} oj_solution;


void init();

void check_arguments();

void parse_arguments(int argc, char *argv[]);

void timeout_hander(int signo);

void print_solution();

void check_spj();

bool check_spj_source(const char *name);

void compile_spj(const char *source, char *target);

int data_filter(const struct dirent *dirp);

void prepare_files(const char *filen_ame, char *infile, char *outfile, char *userfile);

void io_redirect(const char *input_file, const char *stdout_file, const char *stderr_file);

void set_limit(off_t fsize);

void set_compile_limit();

void set_security_option();

int oj_compare_output_spj(const char *file_in,    // std input
                          const char *file_out,   // std output
                          const char *file_user,  // user output
                          const char *spj_exec);  // path of spj
int oj_compare_output(const char *file_out, const char *file_user);

void fix_java_result(const char *stdout_file, const char *stderr_file);

int fix_gcc_result(const char *stderr_file);

void output_acm_result(int result, long time_usage, long memory_usage, int test);

void compile();

void run_solution();

bool judge(const char *input_file, const char *output_file_std, const char *stdout_file_executive,
           const char *stderr_file_executive);

int get_num_of_test();


#endif //POWERJUDGE_JUDGE_H
