#ifndef __JUDGE_H__
#define __JUDGE_H__


// 编译选项
const char* CP_C[] = { "gcc", "-fno-asm", "-lm", "-static", "-Wall",
                       "-O2", "-DONLINE_JUDGE", "-o", "Main", "Main.c", NULL };
const char* CP_CC[] = { "g++", "-fno-asm", "-lm", "-static", "-Wall",
                        "-O2", "-DONLINE_JUDGE", "-o", "Main", "Main.cc", NULL };
const char* CP_PAS[] = { "fpc", "Main.pas",
                         "-O2", "-Co", "-Cr", "-Ct", "-Ci", NULL };
const char* CP_J[] = { "javac", "Main.java", "-encoding", "UTF-8", NULL };
const char* CP_PY[] = { "python","-c","import py_compile; py_compile.compile(r'Main.py')", NULL };


// 配置
char log_file[PATH_SIZE];

char work_dir_root[PATH_SIZE];

char data_dir_root[PATH_SIZE];

// judge本身的时限(ms)
int judge_time_limit            = 15000;

// 编译限制(ms)
int compile_time_limit          = 5000; // HUST is 60s

// 编译限制(MB)
int compile_mem_limit           = 256; // HUST is 2048 MB

// 编译输出限制(MB)
int compile_output_limit        = 256;

// SPJ时间限制(ms)
int spj_time_limit              = 5000;

// 程序运行的栈空间大小(KB)
int stack_size_limit            = 8192; // HUST is 64 MB

// ms
int time_limit_addtion          = 314;

int java_time_factor            = 3;

int java_memory_factor          = 3;

int python_time_factor          = 2;

int python_memory_factor        = 2;
// -- end of configruation --

struct oj_solution_t
{
  int sid;          // solution id
  int pid;          // problem id
  int lang;         // language id
  int time_limit;   // ms
  int memory_limit; // KB
  int output_limit; // B

  int result;
  int status;

  int time_usage;    // ms
  int memory_usage; // KB

  bool spj;

  char work_dir[PATH_SIZE];
  char data_dir[PATH_SIZE];

  char source_file[PATH_SIZE];
  char exec_file[PATH_SIZE];
  char spj_exe_file[PATH_SIZE];

  char stdout_file_executive[PATH_SIZE];
  char stderr_file_executive[PATH_SIZE];

  char stdout_file_spj[PATH_SIZE];
}oj_solution;


void check_arguments();
void parse_arguments(int argc, char *argv[]);
void check_spj();
void init_solution();
void print_solution();
void prepare_files(char *filename, int namelen, 
                   char *infile, char *outfile, char *userfile);
void timeout_hander(int signo);
void io_redirect(const char *input_file, const char *stdout_file_executive);
void set_limit(int fsize);
void set_compile_limit();
void set_security_option();

int strincmp(const char *s1, const char *s2, int n);
int oj_compare_output_spj(
  const char *file_in,  //std input
  const char *file_std, //std output
  const char *file_exec, //user output
  const char *spj_exec,  //path of spj
  const char *file_spj);
int oj_compare_output(const char *file_std, const char *file_exec);
void copy_shell_runtime(const char *work_dir);
void copy_python_runtime(const char *work_dir);
void clean_workdir(const char *work_dir);
void output_result(int result, int time_usage, int memory_usage);

void compile();
int run_solution();
bool judge(const char *input_file, const char *output_file_std, const char *stdout_file_executive);

#endif /* __JUDGE_H__ */