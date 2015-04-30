/*
 * Copyright 2014 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
#ifndef SRC_JUDGE_H_
#define SRC_JUDGE_H_

#include <sys/wait.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <dirent.h>
#include "log.h"
#include "misc.h"
#include "syscalls.h"


// 编译选项
const char* CP_C[] = { "gcc", "-fno-asm", "-lm", "-static", "-std=c99", "-Wall",
                       "-O2", "-DONLINE_JUDGE", "-o", "Main", "Main.c", NULL };
const char* CP_CC[] = { "g++", "-fno-asm", "-lm", "-static", "-Wall",
                        "-O2", "-DONLINE_JUDGE", "-o", "Main", "Main.cc", NULL };
const char* CP_PAS[] = { "fpc", "Main.pas", "-O2", "-Co", "-Cr", "-Ct", "-Ci", NULL };
const char* CP_J[] = { "javac", "Main.java", "-g:none", "-Xlint", "-encoding", "UTF-8", NULL };
const char* CP_PY[] = { "python", "-c", "import py_compile;py_compile.compile(r'Main.py')", NULL };

// "-Xms512m", "-Xmx512m", "-Xss256k"
const char* EXEC_J[] = { "java", "-cp", ".", "-Djava.security.manager",
                         "-Djava.security.policy=../java.policy", "Main", NULL };
const char* EXEC_PY[] = { "python", "Main.py", NULL };


// configruation
// judge本身的时限(ms)
static int judge_time_limit            = 15347;

// 编译限制(ms)
static int compile_time_limit          = 5347;

// 编译限制(MB)
static int compile_memory_limit        = 256;

// 编译输出限制(MB)
static int compile_fsize_limit         = 64;

// SPJ时间限制(ms)
static int spj_time_limit              = 10347;

// 程序运行的栈空间大小(KB)
static int stack_size_limit            = 8192;

// ms
static int time_limit_addtion          = 347;

static int java_time_factor            = 3;

static int java_memory_factor          = 3;

static int python_time_factor          = 2;

static int python_memory_factor        = 2;
/* -- end of configruation -- */

static off_t page_size;

static char work_dir_root[PATH_SIZE] = ".";

static char data_dir_root[PATH_SIZE];

struct oj_solution_t {
  int sid;           // solution id
  int pid;           // problem id
  int lang;          // language id
  int time_limit;    // ms
  int memory_limit;  // KB

  int result;

  int time_usage;    // ms
  int memory_usage;  // KB

  bool spj;

  char work_dir[PATH_SIZE];
  char data_dir[PATH_SIZE];
  char spj_exe_file[PATH_SIZE];
}oj_solution;


static void init(void);
static void check_arguments(void);
static void parse_arguments(int argc, char *argv[]);
static void timeout_hander(int signo);
static void print_solution();

static void check_spj(void);
static int data_filter(const struct dirent *dirp);
static void prepare_files(const char *filen_ame,
                          char *infile,
                          char *outfile,
                          char *userfile);
static void io_redirect(const char *input_file,
                        const char *stdout_file,
                        const char *stderr_file);
static void set_limit(off_t fsize);
static void set_compile_limit(void);
static void set_security_option(void);

static int oj_compare_output_spj(const char *file_in,    // std input
                                 const char *file_out,   // std output
                                 const char *file_user,  // user output
                                 const char *spj_exec);  // path of spj
static int oj_compare_output(const char *file_out, const char *file_user);
static void fix_java_result(const char *stdout_file, const char *stderr_file);
static int fix_gcc_result(const char *stderr_file);
static void output_result(int result, int time_usage, int memory_usage, int test);

static void compile(void);
static void run_solution(void);
static bool judge(const char *input_file,
                  const char *output_file_std,
                  const char *stdout_file_executive,
                  const char *stderr_file_executive);

#endif  // SRC_JUDGE_H_
