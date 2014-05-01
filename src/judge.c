#include "judge.h"

int main(int argc, char *argv[], char *envp[])
{
  nice(10); // 降低优先级

  init();

  parse_arguments(argc, argv);

  if (geteuid() != 0) { // user is not root
    FM_LOG_FATAL("please run as root, or set suid bit(chmod +4755)");
    exit(EXIT_UNPRIVILEGED);
  }

  if (EXIT_SUCCESS != chdir(oj_solution.work_dir)) { // change directory
    FM_LOG_FATAL("chdir(%s) failed, %d: %s",
                   oj_solution.work_dir, errno, strerror(errno));
    exit(EXIT_SET_SECURITY);
  }
  FM_LOG_DEBUG("\n\x1b[31m----- Power Judge 1.0 -----\x1b[0m");

  judge_time_limit += oj_solution.time_limit;
  if (EXIT_SUCCESS != malarm(ITIMER_REAL, judge_time_limit)) {
    FM_LOG_FATAL("set alarm for judge failed, %d: %s", errno, strerror(errno));
    exit(EXIT_VERY_FIRST);
  }
  signal(SIGALRM, timeout_hander);
  
  compile();

  run_solution();

  return 0;
}

void init()
{
  oj_solution.result = OJ_AC;
  oj_solution.time_limit = 1000;
  oj_solution.memory_limit = 65536;
  strcpy(work_dir_root, ".");
  page_size = sysconf(_SC_PAGESIZE);
}

void parse_arguments(int argc, char *argv[])
{
  int opt;
  extern char *optarg;

  while ((opt = getopt(argc, argv, "s:p:t:m:l:d:D:")) != -1) {
    switch (opt) {
      case 's': // Solution ID
        oj_solution.sid               = atoi(optarg);
        break;
      case 'p': // Problem ID
        oj_solution.pid               = atoi(optarg);
        break;
      case 'l': // Language ID
        oj_solution.lang              = atoi(optarg);
        break;
      case 't': // Time limit
        oj_solution.time_limit        = atoi(optarg);
        break;
      case 'm': // Memory limit
        oj_solution.memory_limit      = atoi(optarg);
        break;
      case 'd': // Work directory
        realpath(optarg, work_dir_root);
        break;
      case 'D': // Data directory
        realpath(optarg, data_dir_root);
        break;
      default:
        fprintf(stderr, "unknown option provided: -%c %s", opt, optarg);
        exit(EXIT_BAD_PARAM);
      }
  }
  sprintf(log_file, "%s/oj-judge.log", work_dir_root);
  log_open(log_file);
  
  check_arguments();

  sprintf(oj_solution.work_dir, "%s/%d", work_dir_root, oj_solution.sid);
  sprintf(oj_solution.data_dir, "%s/%d", data_dir_root, oj_solution.pid);

  char source_file[PATH_SIZE];
  sprintf(source_file, "%s/Main.%s", oj_solution.work_dir, lang_ext[oj_solution.lang]);
  if (access(source_file, F_OK ) == -1) {
    FM_LOG_FATAL("Source code file is missing.");
    exit(EXIT_NO_SOURCE_CODE);
  }

  if (oj_solution.lang == LANG_JAVA) {
    oj_solution.memory_limit    *= java_memory_factor;
    oj_solution.time_limit      *= java_time_factor;
  }
  else if (oj_solution.lang == LANG_PYTHON) {
    oj_solution.memory_limit    *= python_memory_factor;
    oj_solution.time_limit      *= python_time_factor;
  }

  print_solution();
}

void check_arguments()
{
  if (oj_solution.sid == 0) {
    FM_LOG_FATAL("Miss parameter: solution id");
    exit(EXIT_MISS_PARAM);
  }
  if (oj_solution.pid == 0) {
    FM_LOG_FATAL("Miss parameter: problem id");
    exit(EXIT_MISS_PARAM);
  }
  if (oj_solution.lang == 0) {
    FM_LOG_FATAL("Miss parameter: language id");
    exit(EXIT_MISS_PARAM);
  }
  if (data_dir_root == NULL) {
    FM_LOG_FATAL("Miss parameter: data directory");
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
  FM_LOG_DEBUG("-- Solution Information --");
  FM_LOG_MONITOR("solution id   %d", oj_solution.sid);
  FM_LOG_TRACE("problem id    %d", oj_solution.pid);
  FM_LOG_TRACE("language      %s", languages[oj_solution.lang]);
  FM_LOG_TRACE("time limit    %d ms", oj_solution.time_limit);
  FM_LOG_TRACE("memory limit  %d KB", oj_solution.memory_limit);
  FM_LOG_DEBUG("work dir      %s", oj_solution.work_dir);
  FM_LOG_DEBUG("data dir      %s", oj_solution.data_dir);
}

void timeout_hander(int signo)
{
  if (signo == SIGALRM) {
    exit(EXIT_TIMEOUT);
  }
}

void compile()
{
  char stdout_compiler[PATH_SIZE];
  char stderr_compiler[PATH_SIZE];
  sprintf(stdout_compiler, "%s/stdout_compiler.txt", oj_solution.work_dir);
  sprintf(stderr_compiler, "%s/stderr_compiler.txt", oj_solution.work_dir);

  pid_t compiler = fork();

  if (compiler < 0) {
    FM_LOG_FATAL("fork compiler failed");
    exit(EXIT_COMPILE_ERROR);
  }
  else if (compiler == 0) {
    // run compiler
    log_add_info("compiler");

    set_compile_limit();

    stdout = freopen(stdout_compiler, "w", stdout);
    stderr = freopen(stderr_compiler, "w", stderr);
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
    FM_LOG_FATAL("execvp compiler error");
    exit(EXIT_COMPILE_ERROR);
  }
  else {
    // Judger
    int status = 0;
    if (waitpid(compiler, &status, WUNTRACED) == -1) {
      FM_LOG_FATAL("waitpid for compiler failed");
      exit(EXIT_COMPILE_ERROR);
    }
    FM_LOG_DEBUG("compiler finished");

    if (oj_solution.lang == LANG_PYTHON && file_size(stderr_compiler) > 0) {
      FM_LOG_TRACE("compile error");
      output_result(OJ_CE, 0, 0);
      exit(EXIT_OK);
    }

    if (WIFEXITED(status)) { // normal termination
      if (EXIT_SUCCESS == WEXITSTATUS(status)) {
        FM_LOG_DEBUG("compile succeeded");
      }
      else if (GCC_COMPILE_ERROR == WEXITSTATUS(status)) {
        FM_LOG_TRACE("compile error");
        output_result(OJ_CE, 0, 0);
        exit(EXIT_OK);
      }
      else {
        FM_LOG_FATAL("compiler unknown exit status %d", WEXITSTATUS(status));
        exit(EXIT_COMPILE_ERROR);
      }
    }
    else {
      if (WIFSIGNALED(status)) { // killed by signal
        FM_LOG_WARNING("compiler limit exceeded");
        output_result(OJ_CE, 0, 0);
        stderr = freopen(stderr_compiler, "a", stderr);
        fprintf(stderr, "Compiler Limit Exceeded\n"); // why ??
        exit(EXIT_OK);
      }
      else if (WIFSTOPPED(status)) { // stopped by signal
        FM_LOG_FATAL("stopped by signal %d\n", WSTOPSIG(status));
      }
      else {
        FM_LOG_FATAL("unknown stop reason, status(%d)", status);
      }
      exit(EXIT_COMPILE_ERROR);
    }
  }
}

void set_compile_limit()
{
  if (oj_solution.lang == LANG_JAVA) return;
  if (oj_solution.lang == LANG_PYTHON) return;

  int cpu_time = compile_time_limit;
  int memory   = compile_memory_limit * STD_MB;
  int fsize    = compile_fsize_limit * STD_MB;

  rlimit lim;

  lim.rlim_cur = lim.rlim_max = cpu_time;
  if (setrlimit(RLIMIT_CPU, &lim) < 0) {
    FM_LOG_FATAL("setrlimit RLIMIT_CPU failed");
    exit(EXIT_SET_LIMIT);
  }

  lim.rlim_cur = lim.rlim_max = memory;
  if (setrlimit(RLIMIT_AS, &lim) < 0) {
    FM_LOG_FATAL("setrlimit RLIMIT_AS failed");
    exit(EXIT_SET_LIMIT);
  }

  lim.rlim_cur = lim.rlim_max = fsize;
  if (setrlimit(RLIMIT_FSIZE, &lim) < 0)
  {
    FM_LOG_FATAL("setrlimit RLIMIT_FSIZE failed");
    exit(EXIT_SET_LIMIT);
  }

  FM_LOG_DEBUG("set compile limit ok");
}

int run_solution()
{
  if (oj_solution.lang == LANG_PYTHON) {
    copy_python_runtime(oj_solution.work_dir);
  }

  DIR *dp;
  dirent *dirp;
  if ((dp = opendir(oj_solution.data_dir)) == NULL) {
    FM_LOG_FATAL("open data directory failed");
    exit(EXIT_PRE_JUDGE_DAA);
  }

  check_spj();

  bool flag = true;
  int num_of_test = 0;
  char input_file[PATH_SIZE];
  char output_file_std[PATH_SIZE];
  char stdout_file_executive[PATH_SIZE];
  char stderr_file_executive[PATH_SIZE];
  sprintf(stderr_file_executive, "%s/stderr_executive.txt", oj_solution.work_dir);

  FM_LOG_DEBUG("start run solution");
  while (flag && (dirp = readdir(dp)) != NULL) {
    int namelen = isInFile(dirp->d_name); // check if the file is *.in
    if (namelen == 0)
      continue;

    // TODO check if the out file is here
    prepare_files( dirp->d_name, namelen, input_file, output_file_std, stdout_file_executive);

    num_of_test++;
    FM_LOG_TRACE("run case: %d", num_of_test);

    flag = judge(input_file, output_file_std, stdout_file_executive, stderr_file_executive);
  }

  if ( oj_solution.lang == LANG_PYTHON) {
    clean_workdir(oj_solution.work_dir);
  }
    
  output_result(oj_solution.result, oj_solution.time_usage, oj_solution.memory_usage);
  closedir(dp);
  return num_of_test;
}

bool judge( const char *input_file, 
            const char *output_file_std, 
            const char *stdout_file_executive, 
            const char *stderr_file_executive )
{
  struct rusage rused;
  pid_t executor = fork();

  if (executor < 0) {
    FM_LOG_FATAL("fork executor failed");
    exit(EXIT_PRE_JUDGE);
  }
  else if (executor == 0) { // child process
    log_add_info("executor");

    long fsize = file_size(output_file_std);
    // io redirect
    io_redirect(input_file, stdout_file_executive, stderr_file_executive);

    // chroot & setuid
    set_security_option();

    // set memory, time and file size limit etc.
    set_limit(fsize); // must after set_security_option()

    FM_LOG_DEBUG("time limit: %d, time usage: %d, time limit addtion: %d",
            oj_solution.time_limit, oj_solution.time_usage,
            time_limit_addtion);
    int real_time_limit = oj_solution.time_limit
                        - oj_solution.time_usage
                        + time_limit_addtion; //time fix
    // set real time alarm
    if (EXIT_SUCCESS != malarm(ITIMER_REAL, real_time_limit)) {
      FM_LOG_FATAL("malarm failed");
      exit(EXIT_PRE_JUDGE);
    }

    FM_LOG_TRACE("begin execute");
    log_close();
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
      exit(EXIT_PRE_JUDGE_PTRACE);
    }

    // load program
    if (oj_solution.lang == LANG_JAVA) {
      execvp(EXEC_J[0], (char * const *) EXEC_J);
    }
    else if (oj_solution.lang == LANG_PYTHON) {
      execv(EXEC_PY[0], (char * const *) EXEC_PY); // execvp is incorrect
    }
    else {
      execl("./Main", "Main", NULL);
    }

    // exec error
    exit(EXIT_PRE_JUDGE_EXECLP);
  }
  else {
    //Judger
    int status          = 0;
    int syscall_id      = 0;
    struct user_regs_struct regs;

    init_syscalls(oj_solution.lang);

    while (true) {
      if (wait4(executor, &status, 0, &rused) < 0) {
        FM_LOG_FATAL("wait4 executor failed, %d:%s", errno, strerror(errno));
        exit(EXIT_JUDGE);
      }

      if (WIFEXITED(status)) {
        if (oj_solution.lang != LANG_JAVA
          || WEXITSTATUS(status) == EXIT_SUCCESS) {
          // AC PE WA
          FM_LOG_TRACE("normal quit");
          int result;
          if (oj_solution.spj) {
            // use SPJ
            result = oj_compare_output_spj( input_file,
                                            output_file_std,
                                            stdout_file_executive,
                                            oj_solution.spj_exe_file);
          }
          else {
            // compare file
            result = oj_compare_output( output_file_std,
                                        stdout_file_executive);
          }
          // WA
          if (result == OJ_WA) {
            oj_solution.result = OJ_WA;
          }
          // AC or PE
          else if (oj_solution.result != OJ_PE) {
            oj_solution.result = result;
          }
          else /* (oj_solution.result == OJ_PE) */ {
            oj_solution.result = OJ_PE;
          }
          FM_LOG_NOTICE("case result: %d, problem result: %d",
                        result, oj_solution.result);
        }
        else {
          // not return 0
          FM_LOG_NOTICE("abnormal quit, exit_code: %d", WEXITSTATUS(status));
          oj_solution.result = OJ_RE;
        }
        break;
      }

      // RE/TLE/OLE
      if ( WIFSIGNALED(status) ||
          (WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP)) {
        int signo = 0;
        if (WIFSIGNALED(status)) {
          signo = WTERMSIG(status);
          FM_LOG_NOTICE("child signaled by %d, %s", signo, strsignal(signo));
        }
        else {
          signo = WSTOPSIG(status);
          FM_LOG_NOTICE("child stopped by %d, %s", signo, strsignal(signo));
        }
        switch (signo)
        {
          // TLE
          case SIGALRM:
          case SIGXCPU:
          case SIGVTALRM:
          case SIGKILL: // During startup program terminated with signal SIGKILL, Killed.
              FM_LOG_TRACE("time limit exceeded");
              oj_solution.result = OJ_TLE;
              break;
          // OLE
          case SIGXFSZ:
              FM_LOG_TRACE("file size limit exceeded");
              oj_solution.result = OJ_OLE;
              break;
          // RE
          case SIGSEGV:
              oj_solution.result = OJ_RE;
              fprintf(stderr, "Segmentation Fault\n");
              break;
          case SIGFPE:
              oj_solution.result = OJ_RE;
              fprintf(stderr, "Floating Point Exception\n");
              break;
          case SIGBUS:
              oj_solution.result = OJ_RE;
              fprintf(stderr, "Bus Error\n");
              break;
          case SIGABRT:
              oj_solution.result = OJ_RE;
              fprintf(stderr, "Abnormal Termination\n");
              break;
          default:
              oj_solution.result = OJ_RE;
              FM_LOG_TRACE("Runtime Error");
              fprintf(stderr, "Other Exception: %s\n", strsignal(signo));
              break;
        } // end of swtich
        ptrace(PTRACE_KILL, executor, NULL, NULL);
        break;
      } // end of  "if (WIFSIGNALED(status) ...)"

      // MLE
      oj_solution.memory_usage = max(oj_solution.memory_usage,
                                      rused.ru_minflt * (page_size / STD_KB));
      // TODO check why memory exceed too much
      //FM_LOG_DEBUG("memory_usage: %d %d %d", oj_solution.memory_usage, rused.ru_minflt, page_size / STD_KB);
      if (oj_solution.memory_usage > oj_solution.memory_limit) {
        oj_solution.result = OJ_MLE;
        FM_LOG_NOTICE("memory limit exceeded: %d (fault: %d * %d)",
                oj_solution.memory_usage, rused.ru_minflt, page_size);
        ptrace(PTRACE_KILL, executor, NULL, NULL);
        break;
      }

      // check syscall
      if (ptrace(PTRACE_GETREGS, executor, NULL, &regs) < 0) {
        FM_LOG_FATAL("ptrace(PTRACE_GETREGS) failed, %d: %s",
                      errno, strerror(errno));
        exit(EXIT_JUDGE);
      }
#ifdef __i386__
      syscall_id = regs.orig_eax;
#else
      syscall_id = regs.orig_rax;
#endif
      if (syscall_id > 0 && !is_valid_syscall(oj_solution.lang, syscall_id)) {
        FM_LOG_NOTICE("restricted function, syscall_id: %d", syscall_id);
        oj_solution.result = OJ_RF;
        ptrace(PTRACE_KILL, executor, NULL, NULL);
        break;
      }

      if (ptrace(PTRACE_SYSCALL, executor, NULL, NULL) < 0) {
        FM_LOG_FATAL("ptrace(PTRACE_SYSCALL) failed");
        exit(EXIT_JUDGE);
      }
    } // end of while
  } // end of fork for judge process

  if ( oj_solution.lang == LANG_PYTHON && file_size(stderr_file_executive)) {
    FM_LOG_TRACE("Runtime Error");
    oj_solution.result = OJ_RE;
  }

  oj_solution.time_usage += ( rused.ru_utime.tv_sec * 1000 +
                              rused.ru_utime.tv_usec / 1000 );

  if (oj_solution.time_usage > oj_solution.time_limit) {
    FM_LOG_TRACE("Time Limit Exceeded");
    oj_solution.result = OJ_TLE;
  }

  if (oj_solution.result != OJ_AC &&
      oj_solution.result != OJ_PE) {
    FM_LOG_NOTICE("not AC/PE, no need to continue");
    if (oj_solution.result == OJ_TLE) {
      oj_solution.time_usage = oj_solution.time_limit;
    }
    if (oj_solution.lang == LANG_JAVA && oj_solution.result == OJ_WA) {
      fix_java_result(stdout_file_executive, stderr_file_executive);
    }
    return false;
  }
  return true;
}

void check_spj()
{
  sprintf(oj_solution.spj_exe_file, "%s/spj", oj_solution.data_dir);
  if (access( oj_solution.spj_exe_file, F_OK ) != -1) {
    // spj file exists
    oj_solution.spj = true;
    FM_LOG_MONITOR("Special Judged");
  }
}

void prepare_files( char *filename, int namelen, 
                    char *infile, char *outfile, char *userfile)
{
  char fname[PATH_SIZE];
  strncpy(fname, filename, namelen);
  fname[namelen] = 0;

  sprintf(infile, "%s/%s.in", oj_solution.data_dir, fname);
  sprintf(outfile, "%s/%s.out", oj_solution.data_dir, fname);
  sprintf(userfile, "%s/%s.out", oj_solution.work_dir, fname);

  FM_LOG_DEBUG("std  input  file: %s", infile);
  FM_LOG_DEBUG("std  output file: %s", outfile);
  FM_LOG_DEBUG("user output file: %s", userfile);
}

void io_redirect( const char *input_file, 
                  const char *stdout_file, 
                  const char *stderr_file )
{
  //io_redirect
  stdin  = freopen(input_file, "r", stdin);
  stdout = freopen(stdout_file, "w", stdout);
  stderr = freopen(stderr_file, "w", stderr);
  if (stdin == NULL || stdout == NULL || stderr == NULL) {
    FM_LOG_FATAL("error freopen: stdin(%p) stdout(%p), stderr(%p)",
                   stdin, stdout, stderr);
    exit(EXIT_PRE_JUDGE);
  }
  FM_LOG_TRACE("io redirect ok!");
}

void set_limit(long fsize)
{
  rlimit lim;

  // Set CPU time limit round up
  lim.rlim_max = (oj_solution.time_limit - oj_solution.time_usage + 999) / 1000 + 1;
  lim.rlim_cur = lim.rlim_max;
  if (setrlimit(RLIMIT_CPU, &lim) < 0) {
    FM_LOG_FATAL("setrlimit RLIMIT_CPU failed");
    exit(EXIT_SET_LIMIT);
  }

  if (oj_solution.lang <= LANG_PASCAL) {
    // Memory control
    lim.rlim_max = (STD_MB << 10) + oj_solution.memory_limit * STD_KB;
    lim.rlim_cur = lim.rlim_max;
    if (setrlimit(RLIMIT_AS, &lim) < 0)
    {
      FM_LOG_FATAL("setrlimit RLIMIT_AS failed");
      exit(EXIT_SET_LIMIT);
    }

    // process control
    lim.rlim_cur = lim.rlim_max = 1;
    if (setrlimit(RLIMIT_NPROC, &lim) < 0) {
      FM_LOG_FATAL("setrlimit RLIMIT_NPROC failed");
      exit(EXIT_SET_LIMIT);
    }
  }

  // Stack space
  lim.rlim_cur = lim.rlim_max = stack_size_limit * STD_KB;
  if (setrlimit(RLIMIT_STACK, &lim) < 0) {
    FM_LOG_FATAL("setrlimit RLIMIT_STACK failed");
    exit(EXIT_SET_LIMIT);
  }

  // Output file size limit
  lim.rlim_cur = lim.rlim_max = fsize + (fsize >> 3) + (STD_MB << 3);
  if (setrlimit(RLIMIT_FSIZE, &lim) < 0) {
    FM_LOG_FATAL("setrlimit RLIMIT_FSIZE failed");
    exit(EXIT_SET_LIMIT);
  }
  FM_LOG_DEBUG("File size limit: %d", lim.rlim_max);

  FM_LOG_TRACE("set execute limit ok");
}

void set_security_option()
{
  struct passwd *nobody = getpwnam("nobody");
  if (nobody == NULL) {
    FM_LOG_FATAL("no user named 'nobody'? %d: %s", errno, strerror(errno));
    exit(EXIT_SET_SECURITY);
  }

  if (oj_solution.lang != LANG_JAVA) {
    char cwd[PATH_SIZE], *tmp = getcwd(cwd, PATH_SIZE-1);
    if (tmp == NULL) {
      FM_LOG_FATAL("getcwd failed, %d: %s", errno, strerror(errno));
      exit(EXIT_SET_SECURITY);
    }
    
    // chroot, current directory will be the root dir
    if (EXIT_SUCCESS != chroot(cwd)) {
      FM_LOG_FATAL("chroot(%s) failed, %d: %s", cwd, errno, strerror(errno));
      exit(EXIT_SET_SECURITY);
    }
  }

  /*if (oj_solution.lang != LANG_JAVA)*/ {
    // setgid, must before setuid
    if (EXIT_SUCCESS != setgid(nobody->pw_gid)) {
      FM_LOG_FATAL("setgid(%d) failed, %d: %s",
                    nobody->pw_gid, errno, strerror(errno));
      exit(EXIT_SET_SECURITY);
    }

    // setuid
    if (EXIT_SUCCESS != setuid(nobody->pw_uid)) {
      FM_LOG_FATAL("setuid(%d) failed, %d: %s",
                    nobody->pw_uid, errno, strerror(errno));
      exit(EXIT_SET_SECURITY);
    }

    // set real, effective and saved user ID
    if (EXIT_SUCCESS != setresuid(nobody->pw_uid, 
                                  nobody->pw_uid, 
                                  nobody->pw_uid)) {
      FM_LOG_FATAL("setresuid(%d) failed, %d: %s",
                     nobody->pw_uid, errno, strerror(errno));
      exit(EXIT_SET_SECURITY);
    }
  }

  FM_LOG_TRACE("set_security_option ok");
}

// Run spj
int oj_compare_output_spj(
              const char *file_in,   // std input
              const char *file_out,  // std output
              const char *file_user, // user output
              const char *spj_exec)  // path of spj
{
  FM_LOG_TRACE("start compare spj");
  pid_t pid_spj = fork();
  if (pid_spj < 0) {
    FM_LOG_FATAL("fork for spj failed");
    exit(EXIT_COMPARE_SPJ);
  }
  else if (pid_spj == 0) { // child process
    log_add_info("spj");

    // Set spj timeout
    if (EXIT_SUCCESS == malarm(ITIMER_REAL, spj_time_limit)) {
      FM_LOG_TRACE("load spj: %s", spj_exec);
      log_close();
      execlp(spj_exec, spj_exec, file_in, file_out, file_user, NULL);
      exit(EXIT_COMPARE_SPJ_FORK);
    }
    else {
      FM_LOG_FATAL("malarm failed");
      exit(EXIT_COMPARE_SPJ);
    }
  }
  else {
    int status = 0;
    if (waitpid(pid_spj, &status, 0) < 0) {
      FM_LOG_FATAL("waitpid for spj failed, %d:%s", errno, strerror(errno));
      exit(EXIT_COMPARE_SPJ);
    }

    if (WIFEXITED(status)) {
      switch (WEXITSTATUS(status)) {
        case SPJ_AC:
          return OJ_AC;
        case SPJ_PE:
          return OJ_PE;
        case SPJ_WA:
          return OJ_WA;
        default:
          return OJ_VE;
      }
    }
    else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
      // recv SIGNALRM
      FM_LOG_WARNING("spj: time out");
    }
    else {
      // spj RE
      FM_LOG_WARNING("unkown termination, status = %d", status);
    }
  }
  return OJ_VE;
}

int oj_compare_output(const char *file_out, const char *file_user)
{
  FM_LOG_TRACE("start compare");
  FILE *fp_std = fopen(file_out, "r");
  if (fp_std == NULL) {
    FM_LOG_FATAL("open standard output failed: %s", file_out);
    exit(EXIT_COMPARE);
  }

  FILE *fp_exe = fopen(file_user, "r");
  if (fp_exe == NULL) {
    FM_LOG_FATAL("open user output failed: %s", file_user);
    exit(EXIT_COMPARE);
  }

  int a, b, Na = 0, Nb = 0;
  enum {
    AC = OJ_AC,
    PE = OJ_PE,
    WA = OJ_WA
  } status = AC;

  while (true) {
    while ((a = fgetc(fp_std)) == '\r'); // \r, \n ??
    while ((b = fgetc(fp_exe)) == '\r');
    Na++, Nb++;

    // deal with '\r' and '\n'
    if (a == '\r') a = '\n';
    if (b == '\r') b = '\n';

    if (feof(fp_std) && feof(fp_exe)) {
      break;
    }
    else if (feof(fp_std) || feof(fp_exe)) {
      // deal with tailing white spaces
      FM_LOG_TRACE("one file ended");
      FILE *fp_tmp;
      if (feof(fp_std)) {
        if (!is_space_char(b)) {
          FM_LOG_TRACE("WA exe['%c':0x%x @%d]", b, b, Nb);
          status = WA;
          break;
        }
        fp_tmp = fp_exe;
      }
      else { /* feof(fp_exe) */
        if (!is_space_char(a)) {
          FM_LOG_TRACE("WA std['%c':0x%x @%d]", a, a, Na);
          status = WA;
          break;
        }
        fp_tmp = fp_std;
      }
      int c;
      while (c = fgetc(fp_tmp), c != EOF) {
        if (c == '\r') c = '\n';
        if (!is_space_char(c)) {
          FM_LOG_TRACE("WA ['%c':0x%x]", c, c);
          status = WA;
          break;
        }
      }
      break;
    }

    if (a != b) {
      status = PE;
      if (is_space_char(a) && is_space_char(b)) {
        continue;
      }
      if (is_space_char(a)) {
        ungetc(b, fp_exe);
        Nb--;
      }
      else if (is_space_char(b)) {
        ungetc(a, fp_std);
        Na--;
      }
      else {
        FM_LOG_TRACE("WA ['%c':0x%x @%d] : ['%c':0x%x @%d]", a, a, Na, b, b, Nb);
        status = WA;
        break;
      }
    }
  } // end of while

  if (status == WA) {
    make_diff_out(fp_std, fp_exe, a, b, oj_solution.work_dir, file_out);
  }
  fclose(fp_std);
  fclose(fp_exe);
  FM_LOG_TRACE("compare finished, result = %s",
               status == AC ? "AC" : (status == PE ? "PE" : "WA"));
  return status;
}

void fix_java_result(const char *stdout_file, const char *stderr_file)
{
  int comp_res = execute_cmd("/bin/grep 'java.lang.OutOfMemoryError' %s", stderr_file);
  if (!comp_res) {
    oj_solution.result = OJ_MLE;
    oj_solution.memory_usage = oj_solution.memory_limit * STD_KB;
    return;
  }

  comp_res = execute_cmd("/bin/grep 'java.lang.OutOfMemoryError' %s", stdout_file);
  if (!comp_res) {
    oj_solution.result = OJ_MLE;
    oj_solution.memory_usage = oj_solution.memory_limit * STD_KB;
    return;
  }

  comp_res = execute_cmd("/bin/grep 'Exception' %s", stderr_file);
  if (!comp_res) {
    oj_solution.result = OJ_RE;
    return;
  }

  comp_res = execute_cmd("/bin/grep 'Could not create' %s", stderr_file);
  if (!comp_res) {
    oj_solution.result = OJ_RE;
    return;
  }
}

//Output result
void output_result(int result, int time_usage, int memory_usage)
{
  FM_LOG_MONITOR("result(%d): %s, time: %d ms, memory: %d KB", 
    result, result_str[result], time_usage, memory_usage);
  // this is judge result for Web app
  printf("%d %d %d\n", result, time_usage, memory_usage);
}
