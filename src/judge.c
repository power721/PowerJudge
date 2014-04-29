#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ptrace.h>
#include "log.h"
#include "misc.h"
#include "judge.h"

int main(int argc, char *argv[], char *envp[])
{
  nice(10); // 降低优先级

  init_solution();

  parse_arguments(argc, argv);
  chdir(oj_solution.work_dir);

  FM_LOG_DEBUG("\n\x1b[31m----- Power Judge 1.0 -----\x1b[0m");
  if (geteuid() != 0) {
    FM_LOG_FATAL("please run as root, or set suid bit(chmod +4755)");
    exit(EXIT_UNPRIVILEGED);
  }

  judge_time_limit += oj_solution.time_limit;
  if (EXIT_SUCCESS != malarm(ITIMER_REAL, judge_time_limit)) {
    FM_LOG_FATAL("set alarm for judge failed, %d: %s", errno, strerror(errno));
    exit(EXIT_VERY_FIRST);
  }
  signal(SIGALRM, timeout_hander);
  
  compile();

  output_result(OJ_AC, 0, 0);

  return 0;
}

void parse_arguments(int argc, char *argv[])
{
  int opt;
  extern char *optarg;

  while ((opt = getopt(argc, argv, "s:p:t:m:l:d:D:")) != -1) {
    switch (opt) {
      case 's': // solution ID
        oj_solution.sid               = atoi(optarg);
        break;
      case 'p': // Problem ID
        oj_solution.pid               = atoi(optarg);
        break;
      case 't': // Time limit
        oj_solution.time_limit        = atoi(optarg);
        break;
      case 'm': // Memory limit
        oj_solution.memory_limit      = atoi(optarg);
        break;
      case 'l': // Language
        oj_solution.lang              = atoi(optarg);
        break;
      case 'd': // Work directory
        strncpy(work_dir_root, optarg, PATH_SIZE);
        break;
      case 'D': // Data directory path
        strncpy(data_dir_root, optarg, PATH_SIZE);
        break;
      default:
        fprintf(stderr, "unknown option provided: -%c %s", opt, optarg);
        exit(EXIT_BAD_PARAM);
      }
  }
  sprintf(log_file, "%s/oj-judge.log", work_dir_root);
  log_open(log_file);
  
  check_arguments();

  realpath(work_dir_root, oj_solution.work_dir);
  realpath(data_dir_root, oj_solution.data_dir);
  sprintf(oj_solution.work_dir, "%s/%d", oj_solution.work_dir, oj_solution.sid);
  sprintf(oj_solution.data_dir, "%s/%d", oj_solution.data_dir, oj_solution.pid);

  sprintf(oj_solution.source_file, "%s/Main.%s", oj_solution.work_dir, lang_ext[oj_solution.lang]);
  if( access( oj_solution.source_file, F_OK ) == -1 ) {
    FM_LOG_FATAL("Source code file is missing.");
    exit(EXIT_NO_SOURCE);
  }
  sprintf(oj_solution.exec_file, "%s/Main", oj_solution.work_dir);
  //oj_solution.output_limit           = filesize(oj_solution.output_file_std);
  if(oj_solution.lang == LANG_JAVA) {
    oj_solution.memory_limit        *= java_memory_factor;
    oj_solution.time_limit          *= java_time_factor;
  }
  else if(oj_solution.lang == LANG_PYTHON) {
    sprintf(oj_solution.exec_file, "%s/Main.py", oj_solution.work_dir);
    oj_solution.memory_limit        *= python_memory_factor;
    oj_solution.time_limit          *= python_time_factor;
  }
  sprintf(oj_solution.stdout_file_compiler, "%s/stdout_compiler.txt", oj_solution.work_dir);
  sprintf(oj_solution.stderr_file_compiler, "%s/stderr_compiler.txt", oj_solution.work_dir);
  sprintf(oj_solution.stderr_file_executive, "%s/stderr_executive.txt", oj_solution.work_dir);
  //oj_solution.stdout_file_executive  = oj_solution.work_dir + "/" + basename(oj_solution.output_file_std);

  print_solution();
}

void init_solution()
{
  oj_solution.result = OJ_WAIT;
  oj_solution.time_limit = 1000;
  oj_solution.memory_limit = 65536;
  strcpy(work_dir_root, ".");
}

void check_arguments()
{
  if (oj_solution.sid == 0) {
    FM_LOG_FATAL("Miss parameter: solution id.");
    exit(EXIT_MISS_PARAM);
  }
  if (oj_solution.pid == 0) {
    FM_LOG_FATAL("Miss parameter: problem id.");
    exit(EXIT_MISS_PARAM);
  }
  if (oj_solution.lang == 0) {
    FM_LOG_FATAL("Miss parameter: language id.");
    exit(EXIT_MISS_PARAM);
  }
  if (data_dir_root == NULL) {
    FM_LOG_FATAL("Miss parameter: data directory.");
    exit(EXIT_MISS_PARAM);
  }

  switch (oj_solution.lang) {
    case LANG_C:
    case LANG_CPP:
    case LANG_PASCAL:
    case LANG_JAVA:
    case LANG_PYTHON:
      break;
    default:
      FM_LOG_FATAL("Unknown language id: %d", oj_solution.lang);
      exit(EXIT_BAD_PARAM);
  }
}

void print_solution()
{
  FM_LOG_DEBUG("--solution information--");
  FM_LOG_DEBUG("solution id   %d", oj_solution.sid);
  FM_LOG_DEBUG("problem id    %d", oj_solution.pid);
  FM_LOG_DEBUG("language      %s", languages[oj_solution.lang]);
  FM_LOG_DEBUG("time limit    %d ms", oj_solution.time_limit);
  FM_LOG_DEBUG("memory limit  %d KB", oj_solution.memory_limit);
  FM_LOG_DEBUG("work dir      %s", oj_solution.work_dir);
  FM_LOG_DEBUG("data dir      %s", oj_solution.data_dir);
  //FM_LOG_DEBUG("spj           %s", spj ? "true" : "false");
  FM_LOG_DEBUG("");
}

bool check_spj()
{
  sprintf(oj_solution.spj_exe_file, "%s/%d/spj", oj_solution.data_dir, oj_solution.pid);
  if( access( oj_solution.spj_exe_file, F_OK ) != -1 ) {
    // file exists
    oj_solution.spj = 1;
    sprintf(oj_solution.stdout_file_spj, "%s/stdout_spj.txt", oj_solution.work_dir);
    return true;
  }
  return false;
}

void timeout_hander(int signo)
{
  if (signo == SIGALRM) {
    exit(EXIT_TIMEOUT);
  }
}

void compile()
{
  //compile
  pid_t compiler = fork();
  int status = 0;
  if (compiler < 0) {
    FM_LOG_FATAL("error fork compiler");
    exit(EXIT_COMPILE_ERROR);
  } else if (compiler == 0) {
    // run compiler
    log_add_info("compiler");
    static char buffer[1024];
    getcwd(buffer, 1024);
    FM_LOG_NOTICE("cwd = %s", buffer);

    set_compile_limit();
    stdout = freopen(oj_solution.stdout_file_compiler, "w", stdout);
    stderr = freopen(oj_solution.stderr_file_compiler, "w", stderr);
    if (stdout == NULL || stderr == NULL) {
      FM_LOG_FATAL("error freopen: stdout(%p), stderr(%p)", stdout, stderr);
      exit(EXIT_COMPILE_ERROR);
    }

    switch (oj_solution.lang) {
      case LANG_C:
        print_compiler(CP_C);
        execvp(CP_C[0], (char * const *) CP_C);
        break;

      case LANG_CPP:
        print_compiler(CP_CC);
        execvp(CP_CC[0], (char * const *) CP_CC);
        break;

      case LANG_PASCAL:
        print_compiler(CP_PAS);
        execvp(CP_PAS[0], (char * const *) CP_PAS);
        break;

      case LANG_JAVA:
        print_compiler(CP_J);
        execvp(CP_J[0], (char * const *) CP_J);
        break;

      case LANG_PYTHON:
        print_compiler(CP_PY);
        execvp(CP_PY[0], (char * const *) CP_PY);
        break;
    }

    // execvp error
    FM_LOG_FATAL("exec error");
    exit(EXIT_COMPILE_ERROR);
  }
  else {
    // Judger
    pid_t w = waitpid(compiler, &status, WUNTRACED);
    if (w == -1) {
      FM_LOG_FATAL("waitpid error");
      exit(EXIT_COMPILE_ERROR);
    }

    FM_LOG_TRACE("compiler finished");
    if (WIFEXITED(status)) {
      if (EXIT_SUCCESS == WEXITSTATUS(status)) {
        FM_LOG_TRACE("compile succeeded");
      }
      else if (GCC_COMPILE_ERROR == WEXITSTATUS(status)) {
        FM_LOG_TRACE("compile error");
        output_result(OJ_CE, 0, 0);
        exit(EXIT_OK);
      }
      else {
        FM_LOG_FATAL(" compiler unknown exit status %d", WEXITSTATUS(status));
        exit(EXIT_COMPILE_ERROR);
      }
    }
    else {
      if (WIFSIGNALED(status)) {
        FM_LOG_WARNING("compiler limit exceeded");
        output_result(OJ_CE, 0, 0);
        stderr = freopen(oj_solution.stderr_file_compiler, "w", stderr);
        fprintf(stderr, "Compiler Limit Exceeded\n");
        exit(EXIT_OK);
      }
      else if (WIFSTOPPED(status)) {
        FM_LOG_WARNING("stopped by signal %d\n", WSTOPSIG(status));
      }
      else {
        FM_LOG_WARNING("unknown stop reason, status(%d)", status);
      }
      exit(EXIT_COMPILE_ERROR);
    }
  }
}

void set_compile_limit()
{
  if(oj_solution.lang == LANG_JAVA) return;
  if(oj_solution.lang == LANG_PYTHON) return;

  int cpu = (compile_time_limit + 999) / 1000;
  int mem = compile_mem_limit * STD_MB;

  rlimit lim;

  lim.rlim_cur = lim.rlim_max = cpu;
  if (setrlimit(RLIMIT_CPU, &lim) < 0) {
    FM_LOG_FATAL("error setrlimit for RLIMIT_CPU");
    exit(EXIT_SET_LIMIT);
  }

  lim.rlim_cur = lim.rlim_max = mem;
  if (setrlimit(RLIMIT_AS, &lim) < 0) {
    perror("setrlimit");
    exit(EXIT_SET_LIMIT);
  }

  /*
  // File size limit control
  lim.rlim_max = compile_output_limit * STD_MB;
  lim.rlim_cur = lim.rlim_max;
  if (setrlimit(RLIMIT_FSIZE, &lim) < 0)
  {
      FM_LOG_WARNING("setrlimit RLIMIT_FSIZE failed");
      exit(EXIT_SET_LIMIT);
  }
  */

  FM_LOG_TRACE("set compile limit ok");
}

//Output result
void output_result(int result, int time_usage, int memory_usage)
{
  FM_LOG_TRACE("result(%d): %s, time: %d ms, memory: %d KB", 
    result, result_str[result], time_usage, memory_usage);
  printf("%d %d %d\n", result, time_usage, memory_usage); // this is judge result for Web app
}