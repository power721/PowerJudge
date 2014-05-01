#ifndef __JUDGE_H__
#define __JUDGE_H__

#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ptrace.h>
#include <dirent.h>
#include "log.h"
#include "misc.h"


// 编译选项
const char* CP_C[] = { "gcc", "-fno-asm", "-lm", "-static", "-Wall",
                       "-O2", "-DONLINE_JUDGE", "-o", "Main", "Main.c", NULL };
const char* CP_CC[] = { "g++", "-fno-asm", "-lm", "-static", "-Wall",
                        "-O2", "-DONLINE_JUDGE", "-o", "Main", "Main.cc", NULL };
const char* CP_PAS[] = { "fpc", "Main.pas", "-O2", "-Co", "-Cr", "-Ct", "-Ci", NULL };
const char* CP_J[] = { "javac", "Main.java", "-encoding", "UTF-8", NULL };
const char* CP_PY[] = { "python", "-c", "import py_compile;py_compile.compile(r'Main.py')", NULL };

// "-Xms512m", "-Xmx512m", "-Xss256k"
const char* EXEC_J[] = { "java", "-Djava.security.manager", 
                         "-Djava.security.policy=../java.policy", "-cp", "./", "Main", NULL };
const char* EXEC_PY[] = { "python", "Main.py", NULL };


// 配置
char log_file[PATH_SIZE];

char work_dir_root[PATH_SIZE];

char data_dir_root[PATH_SIZE];

// judge本身的时限(ms)
int judge_time_limit            = 15347;

// 编译限制(s)
int compile_time_limit          = 5; // HUST is 60s

// 编译限制(MB)
int compile_memory_limit        = 256; // HUST is 2048 MB

// 编译输出限制(MB)
int compile_fsize_limit         = 256;

// SPJ时间限制(ms)
int spj_time_limit              = 5347;

// 程序运行的栈空间大小(KB)
int stack_size_limit            = 8192; // HUST is 64 MB

// ms
int time_limit_addtion          = 347;

int java_time_factor            = 3;

int java_memory_factor          = 3;

int python_time_factor          = 2;

int python_memory_factor        = 2;
/* -- end of configruation -- */

long page_size;

struct oj_solution_t
{
  int sid;          // solution id
  int pid;          // problem id
  int lang;         // language id
  int time_limit;   // ms
  int memory_limit; // KB

  int result;

  int time_usage;    // ms
  int memory_usage;  // KB

  bool spj;

  char work_dir[PATH_SIZE];
  char data_dir[PATH_SIZE];

  char spj_exe_file[PATH_SIZE];
}oj_solution;


void init();
void check_arguments();
void parse_arguments(int argc, char *argv[]);
void timeout_hander(int signo);
void print_solution();
void check_spj();
void prepare_files(char *filen_ame, int namelen, 
                   char *infile, char *outfile, char *userfile);
void io_redirect(const char *input_file, 
                 const char *stdout_file, 
                 const char *stderr_file);
void set_limit(long fsize);
void set_compile_limit();
void set_security_option();

int oj_compare_output_spj(const char *file_in,  //std input
                          const char *file_out, //std output
                          const char *file_user, //user output
                          const char *spj_exec);  //path of spj
int oj_compare_output(const char *file_out, const char *file_user);
void fix_java_result(const char *stdout_file, const char *stderr_file);
void output_result(int result, int time_usage, int memory_usage, int test);

void compile();
void run_solution();
bool judge(const char *input_file, 
           const char *output_file_std, 
           const char *stdout_file_executive, 
           const char *stderr_file_executive);

#endif /* __JUDGE_H__ */