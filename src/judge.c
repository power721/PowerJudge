#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ptrace.h>
#include "judge.h"

int main(int argc, char *argv[], char *envp[])
{
  nice(10); // 降低优先级

  init_solution();

  parse_arguments(argc, argv);

  FM_LOG_DEBUG("\n\x1b[31m-----Power Judge 1.0-----\x1b[0m");
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
  

  if (compile() == EXIT_OK) {

  }

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
  
  if (!check_arguments()) {
    exit(EXIT_MISS_PARAM);
  }

  const char *ext = NULL;
  switch (oj_solution.lang) {
    case LANG_C:
      ext = "c"; break;
    case LANG_CPP:
      ext = "cc"; break;
    case LANG_PASCAL:
      ext = "pas"; break;
    case LANG_JAVA:
      ext = "java"; break;
    case LANG_PYTHON:
      ext = "py"; break;
    default:
      FM_LOG_FATAL("Unknown language id: %d", oj_solution.lang);
      exit(EXIT_BAD_PARAM);
  }
  sprintf(oj_solution.work_dir, "%s/%d", work_dir_root, oj_solution.sid);
  sprintf(oj_solution.data_dir, "%s/%d", data_dir_root, oj_solution.pid);

  sprintf(oj_solution.source_file, "%s/Main.%s", oj_solution.work_dir, ext);
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

  //oj_solution.dump_to_log();
}

void init_solution()
{
  oj_solution.time_limit = 1000;
  oj_solution.memory_limit = 65536;
  strcpy(work_dir_root, ".");
}

bool check_arguments()
{
  if (oj_solution.sid == 0) {
    FM_LOG_FATAL("Miss parameter: solution id.");
    return false;
  }
  if (oj_solution.pid == 0) {
    FM_LOG_FATAL("Miss parameter: problem id.");
    return false;
  }
  if (oj_solution.lang == 0) {
    FM_LOG_FATAL("Miss parameter: language id.");
    return false;
  }
  if (data_dir_root == NULL) {
    FM_LOG_FATAL("Miss parameter: data directory.");
    return false;
  }
  return true;
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

//a simpler interface for setitimer
//which can be ITIMER_REAL, ITIMER_VIRTUAL, ITIMER_PROF
int malarm(int which, int milliseconds) {
  struct itimerval t;
  FM_LOG_TRACE("malarm: %d", milliseconds);
  t.it_value.tv_sec       = milliseconds / 1000;
  t.it_value.tv_usec      = milliseconds % 1000 * 1000;
  t.it_interval.tv_sec    = 0;
  t.it_interval.tv_usec   = 0;
  return setitimer(which, &t, NULL);
}

int compile()
{
  //compile
  pid_t compiler = fork();
  int status = 0;
 if (compiler < 0) {
    FM_LOG_FATAL("error fork compiler");
    exit(EXIT_COMPILE_ERROR);
  }
  else if (compiler == 0) {
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

    //malarm(ITIMER_REAL, compile_time_limit);
    char buff[BUFF_SIZE];
    switch (oj_solution.lang) {
      case LANG_C:
        FM_LOG_TRACE("start compile: gcc -fno-asm -lm -static -Wall -O2 -DONLINE_JUDGE -o %s %s",
                oj_solution.exec_file, oj_solution.source_file);
        execlp("/usr/bin/gcc", "gcc", "-fno-asm", "-lm", "-static", "-Wall", "-O2", "-DONLINE_JUDGE",
               "-o", oj_solution.exec_file,
               oj_solution.source_file,
               NULL);
        break;

      case LANG_CPP:
        FM_LOG_TRACE("start compile: g++ -fno-asm -lm -static -Wall -O2 -DONLINE_JUDGE -o %s %s",
                oj_solution.exec_file, oj_solution.source_file);
        execlp("/usr/bin/g++", "g++", "-fno-asm", "-lm", "-static", "-Wall", "-O2", "-DONLINE_JUDGE",
               "-o", oj_solution.exec_file,
               oj_solution.source_file,
               NULL);
        break;

      case LANG_JAVA:
        FM_LOG_TRACE("start compile: javac %s -d %s -encoding UTF-8", oj_solution.source_file, oj_solution.work_dir);
        execlp("javac", "javac",
               oj_solution.source_file, "-d", oj_solution.work_dir, "-encoding", "UTF-8",
               NULL);
        break;

      case LANG_PASCAL:
        FM_LOG_TRACE("start compile: fpc -o %s %s -Co -Cr -Ct -Ci",
                oj_solution.exec_file, oj_solution.source_file);
        execlp("fpc", "fpc", oj_solution.source_file,
                "-o", oj_solution.exec_file,
                "-Co", "-Cr", "-Ct", "-Ci",
               NULL);
        break;

      case LANG_PYTHON:
        sprintf(buff, "import py_compile; py_compile.compile(r'%s')", oj_solution.source_file);
        FM_LOG_TRACE("start compile: python python -c %s ", buff);
            execlp("python", "python", "-c", buff, NULL);
        break;
    }

    // execlp error
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
  return 0;
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