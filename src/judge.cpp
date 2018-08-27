//
// Created by w703710691d on 18-8-24.
//

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>
#include <csignal>
#include <wait.h>
#include <sys/resource.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <algorithm>
#include "judge.h"
#include "log.h"
#include "misc.h"
#include "syscalls.h"
#include "db_updater.h"

int main(int argc, char *argv[], char *envp[]) {
    if (nice(10) == -1) {  // increase nice value(decrease pripority)
        FM_LOG_WARNING("increase nice value failed: %s", strerror(errno));
    }

    init();

    parse_arguments(argc, argv);

    if (geteuid() == 0) {  // effective user is not root
        FM_LOG_FATAL("please do not run as root, run as judge");
        exit(EXIT_PRIVILEGED);
    }

    if (EXIT_SUCCESS != chdir(oj_solution.work_dir)) {  // change current directory
        FM_LOG_FATAL("chdir(%s) failed: %s", oj_solution.work_dir, strerror(errno));
        exit(EXIT_CHDIR);
    }
    FM_LOG_DEBUG("\n\x1b[31m----- Power Judge 1.0 -----\x1b[0m");

    judge_time_limit += oj_solution.time_limit;
    judge_time_limit *= get_num_of_test();

    if (EXIT_SUCCESS != malarm(ITIMER_REAL, judge_time_limit)) {
        FM_LOG_FATAL("set alarm for judge failed: %s", strerror(errno));
        exit(EXIT_VERY_FIRST);
    }

    if (signal(SIGALRM, timeout_hander) == SIG_ERR) {  // install signal hander for timeout
        FM_LOG_FATAL("cannot handle SIGALRM");
        exit(EXIT_VERY_FIRST);
    }
    init_connet();
    compile();

    run_solution();
    close_connet();
    return 0;
}

void init() {
    oj_solution.result = OJ_AC;  // this is the result of empty data directory
    oj_solution.time_limit = 1000;
    oj_solution.memory_limit = 65536;
    page_size = sysconf(_SC_PAGESIZE);  // get configuration information for page size
}

void parse_arguments(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "s:p:t:m:l:w:d:D:j:c:")) != -1) {
        switch (opt) {
            case 's':  // Solution ID
                oj_solution.sid = atoi(optarg);
                break;
            case 'p':  // Problem ID
                oj_solution.pid = atoi(optarg);
                break;
            case 'l':  // Language ID
                oj_solution.lang = atoi(optarg);
                break;
            case 'j':  // judge type
                oj_solution.judge_type = atoi(optarg);
                break;
            case 't':  // Time limit
                oj_solution.time_limit = (unsigned int) atoi(optarg);
                break;
            case 'm':  // Memory limit
                oj_solution.memory_limit = (unsigned int) atoi(optarg);
                break;
            case 'w':
            case 'd':  // Work directory
                if (realpath(optarg, work_dir_root) == nullptr) {
                    fprintf(stderr, "resolve work dir failed:%s\n", strerror(errno));
                    exit(EXIT_BAD_PARAM);
                }
                break;
            case 'D':  // Data directory
                if (realpath(optarg, data_dir_root) == nullptr) {
                    fprintf(stderr, "resolve data dir failed: %s\n", strerror(errno));
                    exit(EXIT_BAD_PARAM);
                }
                break;
            case 'c':  // Contest ID
                oj_solution.cid = atoi(optarg);
                break;
            default:
                fprintf(stderr, "unknown option provided: -%c %s\n", opt, optarg);
                exit(EXIT_BAD_PARAM);
        }
    }
    char buff[PATH_SIZE];
    snprintf(buff, PATH_SIZE, "%s/oj-judge.log", work_dir_root);
    check_and_rename_log(buff);
    log_open(buff);

    check_arguments();

    snprintf(oj_solution.work_dir, PATH_SIZE, "%s/%d", work_dir_root, oj_solution.sid);
    snprintf(oj_solution.data_dir, PATH_SIZE, "%s/%d", data_dir_root, oj_solution.pid);

    char source_file[PATH_SIZE];
    snprintf(source_file, PATH_SIZE, "%s/Main.%s", oj_solution.work_dir, lang_ext[oj_solution.lang]);
    if (access(source_file, F_OK) == -1) {
        FM_LOG_FATAL("Source code file is missing.");
        exit(EXIT_NO_SOURCE_CODE);
    }

    if (oj_solution.lang == LANG_JAVA) {
        oj_solution.memory_limit *= java_memory_factor;
        oj_solution.time_limit *= java_time_factor;
    } else if (oj_solution.lang == LANG_PYTHON27 || oj_solution.lang == LANG_PYTHON3) {
        oj_solution.memory_limit *= python_memory_factor;
        oj_solution.time_limit *= python_time_factor;
    } else if (oj_solution.lang == LANG_KOTLIN) {
        oj_solution.memory_limit *= kotlin_memory_factor;
        oj_solution.time_limit *= kotlin_time_factor;
    }

    snprintf(buff, PATH_SIZE, "%s/last", work_dir_root);
    unlink(buff);
    if (symlink(oj_solution.work_dir, buff) == -1) {
        FM_LOG_NOTICE("make symlink for %s failed: %s", buff, strerror(errno));
    }

    print_solution();
}

void check_arguments() {
    if (oj_solution.sid == 0) {
        FM_LOG_FATAL("Miss parameter: solution id");
        exit(EXIT_MISS_PARAM);
    }
    if (oj_solution.pid == 0) {
        FM_LOG_FATAL("Miss parameter: problem id");
        exit(EXIT_MISS_PARAM);
    }
    if (strlen(data_dir_root) == 0) {
        FM_LOG_FATAL("Miss parameter: data directory");
        exit(EXIT_MISS_PARAM);
    }

    switch (oj_solution.lang) {
        case LANG_C99:
        case LANG_C11:
        case LANG_CPP98:
        case LANG_CPP11:
        case LANG_CPP14:
        case LANG_CPP17:
        case LANG_PASCAL:
        case LANG_JAVA:
        case LANG_PYTHON27:
        case LANG_PYTHON3:
        case LANG_KOTLIN:
            break;
        default:
            FM_LOG_FATAL("Unknown language id: %d", oj_solution.lang);
            exit(EXIT_BAD_PARAM);
    }
}

void print_solution() {
    FM_LOG_DEBUG("-- Solution Information --");
    FM_LOG_MONITOR("solution id %d", oj_solution.sid);
    FM_LOG_TRACE("problem id    %d", oj_solution.pid);
    FM_LOG_TRACE("language(%d)   %s", oj_solution.lang, languages[oj_solution.lang]);
    FM_LOG_TRACE("time limit    %d ms", oj_solution.time_limit);
    FM_LOG_TRACE("memory limit  %d KB", oj_solution.memory_limit);
    FM_LOG_DEBUG("work dir      %s", oj_solution.work_dir);
    FM_LOG_DEBUG("data dir      %s", oj_solution.data_dir);
}

void timeout_hander(int signo) {
    if (signo == SIGALRM) {
        FM_LOG_FATAL("Judge Timeout");
        exit(EXIT_TIMEOUT);
    }
}

/*
 * #error "This make CE"
 * #warning "Just warning message"
 * #include </dev/core>
 * #include </dev/zero>
 * #include </dev/random>
 * #include </etc/passwd>
 * #include <../../../etc/passwd>
 * egrep '^\s*#include\s*[<"][./].*[>"]' Main.cc
 */
void compile() {
    update_solution_status(oj_solution.cid, oj_solution.sid, OJ_COM, 0);
    char stdout_compiler[PATH_SIZE];
    char stderr_compiler[PATH_SIZE];
    snprintf(stdout_compiler, PATH_SIZE, "%s/stdout_compiler.txt", oj_solution.work_dir);
    snprintf(stderr_compiler, PATH_SIZE, "%s/stderr_compiler.txt", oj_solution.work_dir);

    pid_t compiler = fork();  // create a child process for compiler

    if (compiler < 0) {
        FM_LOG_FATAL("fork compiler failed: %s", strerror(errno));
        exit(EXIT_FORK_COMPILER);
    } else if (compiler == 0) {
        // child process: run compiler
        log_add_info("compiler");

        set_compile_limit();

        stdout = freopen(stdout_compiler, "w", stdout);
        stderr = freopen(stderr_compiler, "w", stderr);
        if (stdout == nullptr || stderr == nullptr) {
            FM_LOG_FATAL("error freopen: stdout(%p), stderr(%p)", stdout, stderr);
            exit(EXIT_COMPILE_IO);
        }
        print_user_group();
        print_word_dir();

        switch (oj_solution.lang) {
            case LANG_C99:
                print_compiler(CP_C99);
                execvp(CP_C99[0], (char *const *) CP_C99);
                break;
            case LANG_C11:
                print_compiler(CP_C11);
                execvp(CP_C11[0], (char *const *) CP_C11);
                break;
            case LANG_CPP98:
                print_compiler(CP_CC98);
                execvp(CP_CC98[0], (char *const *) CP_CC98);
                break;
            case LANG_CPP11:
                print_compiler(CP_CC11);
                execvp(CP_CC11[0], (char *const *) CP_CC11);
                break;
            case LANG_CPP14:
                print_compiler(CP_CC14);
                execvp(CP_CC14[0], (char *const *) CP_CC14);
                break;
            case LANG_CPP17:
                print_compiler(CP_CC98);
                execvp(CP_CC98[0], (char *const *) CP_CC17);
                break;
            case LANG_PASCAL:
                print_compiler(CP_PAS);
                execvp(CP_PAS[0], (char *const *) CP_PAS);
                break;
            case LANG_JAVA:
                print_compiler(CP_J);
                execvp(CP_J[0], (char *const *) CP_J);
                break;
            case LANG_PYTHON27:
                print_compiler(CP_PY27);
                execvp(CP_PY27[0], (char *const *) CP_PY27);
                break;
            case LANG_PYTHON3:
                print_compiler(CP_PY3);
                execvp(CP_PY3[0], (char *const *) CP_PY3);
                break;
            case LANG_KOTLIN:
                print_compiler(CP_KT);
                execvp(CP_KT[0], (char *const *) CP_KT);
                break;
            default:
                FM_LOG_FATAL("Unknown language %d", oj_solution.lang);
                break;
        }

        // execvp error
        FM_LOG_FATAL("execvp compiler error");
        exit(EXIT_COMPILE_EXEC);
    } else {
        // parent process: Judger
        int status = 0;
        if (waitpid(compiler, &status, WUNTRACED) == -1) {
            FM_LOG_FATAL("waitpid for compiler failed: %s", strerror(errno));
            exit(EXIT_COMPILE_ERROR);  // SE
        }
        FM_LOG_DEBUG("compiler finished");

        if ((oj_solution.lang == LANG_PYTHON27 || oj_solution.lang == LANG_PYTHON3) && file_size(stderr_compiler)) {
            FM_LOG_TRACE("compile error");
            output_acm_result(OJ_CE, 0, 0, 0);
            exit(EXIT_OK);
        }

        if (WIFEXITED(status)) {  // normal termination
            if (EXIT_SUCCESS == WEXITSTATUS(status)) {
                FM_LOG_DEBUG("compile succeeded");
            } else if (GCC_COMPILE_ERROR == WEXITSTATUS(status)) {
                FM_LOG_TRACE("compile error");
                output_acm_result(OJ_CE, 0, 0, 0);
                exit(EXIT_OK);
            } else {
                if (fix_gcc_result(stderr_compiler)) {
                    FM_LOG_WARNING("Compiler Limit Exceeded!");
                    output_acm_result(OJ_CE, 0, 0, 0);
                    exit(EXIT_OK);
                } else {
                    FM_LOG_FATAL("compiler unknown exit status %d", WEXITSTATUS(status));
                    output_acm_result(OJ_CE, 0, 0, 0);
                    exit(EXIT_COMPILE_ERROR);
                }
            }
        } else {
            if (WIFSIGNALED(status)) {  // killed by signal
                int signo = WTERMSIG(status);
                FM_LOG_WARNING("Compiler Limit Exceeded: %s", strsignal(signo));
                output_acm_result(OJ_CE, 0, 0, 0);
                stderr = freopen(stderr_compiler, "w", stderr);
                fprintf(stderr, "Compiler Limit Exceeded: %s\n", strsignal(signo));
                exit(EXIT_OK);
            } else if (WIFSTOPPED(status)) {  // stopped by signal
                int signo = WSTOPSIG(status);
                FM_LOG_FATAL("stopped by signal: %s\n", strsignal(signo));
            } else {
                FM_LOG_FATAL("unknown stop reason, status(%d)", status);
            }
            exit(EXIT_COMPILE_ERROR);  // SE
        }
    }
}

void set_compile_limit() {
    if (oj_solution.lang == LANG_JAVA || oj_solution.lang == LANG_PYTHON27 || oj_solution.lang == LANG_PYTHON3 ||
        oj_solution.lang == LANG_KOTLIN)
        return;

    rlimit lim{};
    lim.rlim_cur = lim.rlim_max = compile_time_limit / 1000;
    if (setrlimit(RLIMIT_CPU, &lim) < 0) {
        FM_LOG_FATAL("setrlimit RLIMIT_CPU failed: %s", strerror(errno));
        exit(EXIT_SET_LIMIT);
    }

    if (EXIT_SUCCESS != malarm(ITIMER_REAL, compile_time_limit)) {
        FM_LOG_FATAL("malarm for compiler failed: %s", strerror(errno));
        exit(EXIT_SET_LIMIT);
    }

    lim.rlim_cur = lim.rlim_max = compile_memory_limit * STD_MB;
    if (setrlimit(RLIMIT_AS, &lim) < 0) {
        FM_LOG_FATAL("setrlimit RLIMIT_AS failed: %s", strerror(errno));
        exit(EXIT_SET_LIMIT);
    }

    lim.rlim_cur = lim.rlim_max = compile_fsize_limit * STD_MB;
    if (setrlimit(RLIMIT_FSIZE, &lim) < 0) {
        FM_LOG_FATAL("setrlimit RLIMIT_FSIZE failed: %s", strerror(errno));
        exit(EXIT_SET_LIMIT);
    }

    FM_LOG_DEBUG("set compile limit ok");
}

void run_solution() {
    FM_LOG_DEBUG("run_solution");
#ifndef FAST_JUDGE
    if (oj_solution.lang == LANG_PYTHON27) {
        copy_python_runtime_python2(oj_solution.work_dir);
        FM_LOG_DEBUG("copy_python_runtime");
    } else if (oj_solution.lang == LANG_PYTHON3) {
        copy_python_runtime_python3(oj_solution.work_dir);
        FM_LOG_DEBUG("copy_python_runtime");
    }
#endif

    check_spj();

    struct dirent **namelist;
    int num_of_test;

    num_of_test = scandir(oj_solution.data_dir, &namelist, data_filter, alphasort);
    if (num_of_test < 0) {
        FM_LOG_FATAL("scan data directory failed: %s", strerror(errno));
        exit(EXIT_PRE_JUDGE_DAA);
    }

    int first_failed_test = 0;
    char input_file[PATH_SIZE];
    char output_file_std[PATH_SIZE];
    char stdout_file_executive[PATH_SIZE];
    char stderr_file_executive[PATH_SIZE];

    snprintf(stderr_file_executive, PATH_SIZE, "%s/stderr_executive.txt", oj_solution.work_dir);
    FM_LOG_DEBUG("start run solution (%d cases)", num_of_test);

    for (int i = 0; i < num_of_test; ++i) {
        update_solution_status(oj_solution.cid, oj_solution.sid, OJ_RUN, i + 1);

        prepare_files(namelist[i]->d_name, input_file, output_file_std, stdout_file_executive);

        FM_LOG_TRACE("run case: %d", i + 1);

        bool result = judge(input_file, output_file_std, stdout_file_executive, stderr_file_executive);

        if (oj_solution.result != OJ_AC && !first_failed_test) {
            first_failed_test = i + 1;
        }

        if (!result && oj_solution.judge_type == ACM) {
            break;
        }
    }

    for (int i = 0; i < num_of_test; ++i) {
        free(namelist[i]);
    }
    free(namelist);

#ifndef FAST_JUDGE
    if (oj_solution.lang == LANG_PYTHON27 || oj_solution.lang == LANG_PYTHON3) {
        clean_workdir(oj_solution.work_dir);
    }
#endif

    output_acm_result(oj_solution.result, oj_solution.time_usage, oj_solution.memory_usage, first_failed_test);
}

bool judge(const char *input_file,
           const char *output_file_std,
           const char *stdout_file_executive,
           const char *stderr_file_executive) {
    rusage rused{};
    pid_t executor = fork();  // create a child process for executor

    if (executor < 0) {
        FM_LOG_FATAL("fork executor failed: %s", strerror(errno));
        exit(EXIT_PRE_JUDGE);
    } else if (executor == 0) {  // child process
        log_add_info("executor");

        off_t fsize = file_size(output_file_std);

        // io redirect, must before set_security_option()
        io_redirect(input_file, stdout_file_executive, stderr_file_executive);

        // chroot & setuid
        set_security_option();

        // set memory, time and file size limit etc.
        set_limit(fsize);  // must after set_security_option()

        FM_LOG_DEBUG("time limit: %d, time limit addtion: %d",
                     oj_solution.time_limit, time_limit_addtion);


        uint64_t real_time_limit = oj_solution.time_limit + time_limit_addtion;  // time fix
        // set real time alarm
        if (EXIT_SUCCESS != malarm(ITIMER_REAL, real_time_limit)) {
            FM_LOG_FATAL("malarm for executor failed: %s", strerror(errno));
            exit(EXIT_PRE_JUDGE);
        }

        FM_LOG_TRACE("begin execute");

        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            FM_LOG_FATAL("Trace executor failed: %s", strerror(errno));
            exit(EXIT_PRE_JUDGE_PTRACE);
        }

        // load program
        if (oj_solution.lang == LANG_JAVA) {
            print_executor(EXEC_J);
            execvp(EXEC_J[0], (char *const *) EXEC_J);
        } else if (oj_solution.lang == LANG_KOTLIN) {
            print_executor(EXEC_KT);
            execvp(EXEC_KT[0], (char *const *) EXEC_KT);
        } else if (oj_solution.lang == LANG_PYTHON27) {
            print_executor(EXEC_PY27);
#ifdef FAST_JUDGE
            execvp(EXEC_PY27[0], (char * const *) EXEC_PY27);
#else
            execv(EXEC_PY27[0], (char *const *) EXEC_PY27);
#endif
        } else if (oj_solution.lang == LANG_PYTHON3) {
            print_executor(EXEC_PY3);
#ifdef FAST_JUDGE
            execvp(EXEC_PY3[0], (char * const *) EXEC_PY3);
#else
            execv(EXEC_PY3[0], (char *const *) EXEC_PY3);
#endif
        } else {
            execl("./Main", "./Main", NULL);
        }

        // exec error
        FM_LOG_FATAL("exec error");
        exit(EXIT_PRE_JUDGE_EXECLP);
    } else {
        // Judger
        int status = 0;
        user_regs_struct regs{};
        stderr = freopen("error.txt", "a+", stderr);

        init_syscalls(oj_solution.lang);

        while (true) {
            if (wait4(executor, &status, 0, &rused) < 0) {
                FM_LOG_FATAL("wait4 executor failed: %s", strerror(errno));
                kill(executor, SIGKILL);
                exit(EXIT_JUDGE);
            }

            if (WIFEXITED(status)) {
                if ((oj_solution.lang != LANG_JAVA && oj_solution.lang != LANG_KOTLIN) ||
                    WEXITSTATUS(status) == EXIT_SUCCESS) {
                    // AC PE WA
                    FM_LOG_TRACE("normal quit");
                    int result;
                    if (oj_solution.spj) {
                        // use SPJ
                        result = oj_compare_output_spj(input_file, output_file_std, stdout_file_executive,
                                                       oj_solution.spj_exe_file);
                    } else {
                        // compare file
                        result = oj_compare_output(output_file_std, stdout_file_executive);
                    }
                    // WA
                    if (result == OJ_WA) {
                        oj_solution.result = OJ_WA;
                    } else if (oj_solution.result != OJ_PE) {  // previous case is AC
                        oj_solution.result = result;  // AC or PE
                    } else /* (oj_solution.result == OJ_PE) */ {  // previous case is PE
                        oj_solution.result = OJ_PE;
                    }
                    FM_LOG_NOTICE("case result: %d, problem result: %d", result, oj_solution.result);
                } else {
                    // not return 0
                    oj_solution.result = OJ_RE;
                    FM_LOG_NOTICE("abnormal quit, exit_code: %d", WEXITSTATUS(status));
                }
                break;
            }

            // RE/TLE/OLE
            if (WIFSIGNALED(status) || (WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP)) {
                int signo = 0;
                if (WIFSIGNALED(status)) {
                    signo = WTERMSIG(status);
                    FM_LOG_NOTICE("child process killed by signal %d, %s", signo, strsignal(signo));
                } else {
                    signo = WSTOPSIG(status);
                    FM_LOG_NOTICE("child process stopped by signal %d, %s", signo, strsignal(signo));
                }
                switch (signo) {
                    // Ignore
                    case SIGCHLD:
                        oj_solution.result = OJ_AC;
                        break;
                        // TLE
                    case SIGALRM:    // alarm() and setitimer(ITIMER_REAL)
                    case SIGVTALRM:  // setitimer(ITIMER_VIRTUAL)
                    case SIGXCPU:    // exceeds soft processor limit
                        oj_solution.result = OJ_TLE;
                        FM_LOG_TRACE("Time Limit Exceeded: %s", strsignal(signo));
                        break;
                        // OLE
                    case SIGXFSZ:  // exceeds file size limit
                        oj_solution.result = OJ_OLE;
                        FM_LOG_TRACE("Output Limit Exceeded");
                        break;
                        // RE
                    case SIGSEGV:  // segmentation violation
                    case SIGFPE:   // any arithmetic exception
                    case SIGBUS:   // the process incurs a hardware fault
                    case SIGABRT:  // abort() function
                    case SIGKILL:  // exceeds hard processor limit
                    default:
                        oj_solution.result = OJ_RE;
                        FILE *fp = fopen(stderr_file_executive, "a+");
                        if (fp == nullptr) {
                            fprintf(stderr, "%s\n", strsignal(signo));
                            FM_LOG_WARNING("Runtime Error: %s", strsignal(signo));
                        } else {
                            fprintf(fp, "%s\n", strsignal(signo));
                            fclose(fp);
                        }
                        break;
                }  // end of swtich
                kill(executor, SIGKILL);
                break;
            }  // end of  "if (WIFSIGNALED(status) ...)"

            oj_solution.memory_usage = std::max(oj_solution.memory_usage, (unsigned long) rused.ru_maxrss);
            // TODO(power): check why memory exceed too much
            if (oj_solution.memory_usage > oj_solution.memory_limit) {
                oj_solution.result = OJ_MLE;
                kill(executor, SIGKILL);
                break;
            }

            // check syscall
            if (ptrace(PTRACE_GETREGS, executor, NULL, &regs) < 0) {
                FM_LOG_FATAL("ptrace(PTRACE_GETREGS) failed: %s", strerror(errno));
                kill(executor, SIGKILL);
                exit(EXIT_JUDGE);
            }
            int syscall_id = 0;
#ifdef __i386__
            syscall_id = (int)regs.orig_eax;
#else
            syscall_id = (int) regs.orig_rax;
#endif
            if (syscall_id > 0 && !is_valid_syscall(syscall_id)) {
                oj_solution.result = OJ_RF;
                FM_LOG_FATAL("restricted function, syscall_id: %d", syscall_id);
                kill(executor, SIGKILL);
                break;
            }

            if (ptrace(PTRACE_SYSCALL, executor, NULL, NULL) < 0) {
                FM_LOG_FATAL("ptrace(PTRACE_SYSCALL) failed: %s", strerror(errno));
                kill(executor, SIGKILL);
                exit(EXIT_JUDGE);
            }
        }  // end of while
    }  // end of fork for judge process

    oj_solution.memory_usage = std::max(oj_solution.memory_usage, (unsigned long) rused.ru_maxrss);
    if (oj_solution.memory_usage > oj_solution.memory_limit) {
        oj_solution.result = OJ_MLE;
        FM_LOG_NOTICE("memory limit exceeded: %d (fault: %d * %d)",
                      oj_solution.memory_usage, rused.ru_minflt, page_size);
    }

    oj_solution.time_usage = std::max(oj_solution.time_usage,
                                      (unsigned long) rused.ru_utime.tv_sec * 1000 + rused.ru_utime.tv_usec / 1000);

    if (oj_solution.time_usage > oj_solution.time_limit) {
        oj_solution.result = OJ_TLE;
        FM_LOG_TRACE("Time Limit Exceeded");
    }

    if (oj_solution.result != OJ_AC) {
        if (oj_solution.judge_type == ACM) {
            FM_LOG_NOTICE("not AC/PE, no need to continue");
        }
        if (oj_solution.result == OJ_TLE) {
            oj_solution.time_usage = oj_solution.time_limit;
        } else if (oj_solution.result == OJ_WA) {
            if (oj_solution.lang == LANG_JAVA) { // TODO: kotlin
                fix_java_result(stdout_file_executive, stderr_file_executive);
            } else if ((oj_solution.lang == LANG_PYTHON27 || oj_solution.lang == LANG_PYTHON3) &&
                       file_size(stderr_file_executive)) {
                oj_solution.result = OJ_RE;
                FM_LOG_TRACE("Runtime Error");
            }
        }
        return false;
    }
    return true;
}

void check_spj() {
    snprintf(oj_solution.spj_exe_file, PATH_SIZE, "%s/spj", oj_solution.data_dir);
    if (access(oj_solution.spj_exe_file, F_OK) != -1) {  // spj file exists
        oj_solution.spj = true;
        FM_LOG_MONITOR("Special Judged: %s", oj_solution.spj_exe_file);
    } else {
        if (!check_spj_source("spj.cc")) {
            check_spj_source("spj.c");
        }
    }
}

bool check_spj_source(const char *name) {
    char buffer[PATH_SIZE];
    snprintf(buffer, PATH_SIZE, "%s/%s", oj_solution.data_dir, name);
    if (access(buffer, F_OK) != -1) {
        compile_spj(buffer, oj_solution.spj_exe_file);
        if (access(oj_solution.spj_exe_file, F_OK) != -1) {
            oj_solution.spj = true;
            FM_LOG_MONITOR("Special Judged: %s", oj_solution.spj_exe_file);
            return true;
        }
    }
    return false;
}

void compile_spj(const char *source, char *target) {
    int status = execute_cmd("g++ -lm -static -w -std=gnu++17 -O4 -o %s %s", target, source);
    if (status == -1) {
        FM_LOG_WARNING("compile spj failed: %s", strerror(errno));
    }
}

int data_filter(const struct dirent *dirp) {
    size_t namelen = checkInFile(dirp->d_name);  // check if the file is *.in
    if (namelen == 0)
        return 0;

    if (oj_solution.spj)  // spj may don't need out file
        return 1;

    char fname[PATH_SIZE];
    char outfile[PATH_SIZE];

    strncpy(fname, dirp->d_name, namelen);
    fname[namelen] = 0;
    snprintf(outfile, PATH_SIZE, "%s/%s.out", oj_solution.data_dir, fname);

    if (access(outfile, F_OK) != -1)  // out file exists
        return 1;
    return 0;
}

void prepare_files(const char *filename, char *infile, char *outfile, char *userfile) {
    size_t namelen = strlen(filename) - 3;
    char fname[PATH_SIZE];
    strncpy(fname, filename, namelen);
    fname[namelen] = 0;

    snprintf(infile, PATH_SIZE, "%s/%s.in", oj_solution.data_dir, fname);
    snprintf(outfile, PATH_SIZE, "%s/%s.out", oj_solution.data_dir, fname);
    snprintf(userfile, PATH_SIZE, "%s/%s.out", oj_solution.work_dir, fname);

    char buff[PATH_SIZE];
    snprintf(buff, PATH_SIZE, "%s/%s.in", oj_solution.work_dir, fname);
    if (symlink(infile, buff) == -1) {
        FM_LOG_NOTICE("make symlink for %s failed: %s", buff, strerror(errno));
    }

    FM_LOG_DEBUG("std  input  file: %s", infile);
    FM_LOG_DEBUG("std  output file: %s", outfile);
    FM_LOG_DEBUG("user output file: %s", userfile);
}

void io_redirect(const char *input_file, const char *stdout_file, const char *stderr_file) {
    // io_redirect
    stdin = freopen(input_file, "r", stdin);
    stdout = freopen(stdout_file, "w", stdout);
    stderr = freopen(stderr_file, "a+", stderr);

    if (stdin == nullptr || stdout == nullptr || stderr == nullptr) {
        FM_LOG_FATAL("error freopen: stdin(%p) stdout(%p), stderr(%p)", stdin, stdout, stderr);
        exit(EXIT_PRE_JUDGE);
    }
    FM_LOG_TRACE("io redirect ok!");
}

void set_limit(off_t fsize) {
    rlimit lim{};

    // Set CPU time limit round up, raise SIGXCPU
    lim.rlim_max = (oj_solution.time_limit + 999) / 1000 + 1;
    lim.rlim_cur = lim.rlim_max;
    if (setrlimit(RLIMIT_CPU, &lim) < 0) {
        FM_LOG_FATAL("setrlimit RLIMIT_CPU failed: %s", strerror(errno));
        exit(EXIT_SET_LIMIT);
    }

    if (oj_solution.lang <= LANG_PASCAL) {
        // Memory control, raise SIGSEGV
        lim.rlim_cur = lim.rlim_max = (STD_MB << 10) + oj_solution.memory_limit * STD_KB;
        if (setrlimit(RLIMIT_AS, &lim) < 0) {
            FM_LOG_FATAL("setrlimit RLIMIT_AS failed: %s", strerror(errno));
            exit(EXIT_SET_LIMIT);
        }
    }

    // Stack space, raise SIGSEGV
    lim.rlim_cur = lim.rlim_max = stack_size_limit * STD_KB;
    if (setrlimit(RLIMIT_STACK, &lim) < 0) {
        FM_LOG_FATAL("setrlimit RLIMIT_STACK failed: %s", strerror(errno));
        exit(EXIT_SET_LIMIT);
    }

    // Output file size limit, raise SIGXFSZ
    lim.rlim_cur = lim.rlim_max = (rlim_t) (4 * MAX_LOG_FILE_SIZE);
    if (setrlimit(RLIMIT_FSIZE, &lim) < 0) {
        FM_LOG_FATAL("setrlimit RLIMIT_FSIZE failed: %s", strerror(errno));
        exit(EXIT_SET_LIMIT);
    }
    FM_LOG_DEBUG("File size limit: %d", lim.rlim_max);

    FM_LOG_TRACE("set execute limit ok");
}

void set_security_option() {
    if (oj_solution.lang != LANG_JAVA && oj_solution.lang != LANG_KOTLIN
#ifdef FAST_JUDGE
        && oj_solution.lang != LANG_PYTHON3 && oj_solution.lang != LANG_PYTHON27
#endif
            ) {
        char cwd[PATH_SIZE];
        char *tmp = getcwd(cwd, PATH_SIZE - 1);
        if (tmp == nullptr) {
            FM_LOG_FATAL("getcwd failed: %s", strerror(errno));
            exit(EXIT_SET_SECURITY);
        }

        // chroot, current directory will be the root dir
        if (EXIT_SUCCESS != chroot(cwd)) {
            FM_LOG_FATAL("chroot(%s) failed: %s", cwd, strerror(errno));
            exit(EXIT_SET_SECURITY);
        }
        FM_LOG_DEBUG("chroot(%s)", cwd);
    }

    FM_LOG_TRACE("set_security_option ok");
}

// Run spj
int oj_compare_output_spj(const char *file_in,    // std input file
                          const char *file_out,   // std output file
                          const char *file_user,  // user output file
                          const char *spj_exec)   // path of spj
{
    FM_LOG_TRACE("start compare spj");

    pid_t pid_spj = fork();  // create a child process for spj
    if (pid_spj < 0) {
        FM_LOG_FATAL("fork for spj failed: %s", strerror(errno));
        exit(EXIT_COMPARE_SPJ);
    } else if (pid_spj == 0) {  // child process
        log_add_info("spj");

        // Set spj timeout
        if (EXIT_SUCCESS == malarm(ITIMER_REAL, spj_time_limit)) {
            FM_LOG_TRACE("load spj: %s", spj_exec);
            execlp(spj_exec, spj_exec, file_in, file_out, file_user, NULL);
            FM_LOG_FATAL("execute spj failed");
            exit(EXIT_COMPARE_SPJ_FORK);
        } else {
            FM_LOG_FATAL("malarm for spj failed: %s", strerror(errno));
            exit(EXIT_COMPARE_SPJ);
        }
    } else {
        int status = 0;
        if (waitpid(pid_spj, &status, 0) < 0) {
            FM_LOG_FATAL("waitpid for spj failed: %s", strerror(errno));
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
        } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
            // recv SIGNALRM
            FM_LOG_WARNING("spj: time out");
        } else {
            // spj RE
            FM_LOG_WARNING("unkown termination, status = %d", status);
        }
    }
    return OJ_VE;
}

int oj_compare_output(const char *file_out, const char *file_user) {
    FM_LOG_TRACE("start compare");
    FILE *fp_std = fopen(file_out, "r");
    if (fp_std == nullptr) {
        FM_LOG_FATAL("open standard output file (%s) failed: %s", file_out, strerror(errno));
        exit(EXIT_COMPARE);
    }

    FILE *fp_exe = fopen(file_user, "r");
    if (fp_exe == nullptr) {
        FM_LOG_FATAL("open user output file (%s) failed: %s", file_user, strerror(errno));
        exit(EXIT_COMPARE);
    }

    int a, b, Na = 0, Nb = 0;
    enum {
        AC = OJ_AC,
        PE = OJ_PE,
        WA = OJ_WA
    } status = AC;

    while (true) {
        /*
         * Windows / DOS uses '\r\n';
         * Unix / Linux / OS X uses '\n';
         * Macs before OS X use '\r';
         * TODO(power): out file with '\r' line ending get incorrect PE
         */
        while ((a = fgetc(fp_std)) == '\r') {}
        while ((b = fgetc(fp_exe)) == '\r') {}
        Na++, Nb++;

        // deal with '\r' and '\n'
        if (a == '\r') a = '\n';
        if (b == '\r') b = '\n';

        if (feof(fp_std) && feof(fp_exe)) {
            break;
        } else if (feof(fp_std) || feof(fp_exe)) {
            // deal with tailing white spaces
            FILE *fp_tmp;
            if (feof(fp_std)) {
                FM_LOG_TRACE("std out file ended");
                if (!is_space_char(b)) {
                    FM_LOG_TRACE("WA exe['%c':0x%x @%d]", b, b, Nb);
                    status = WA;
                    break;
                }
                fp_tmp = fp_exe;
            } else { /* feof(fp_exe) */
                FM_LOG_TRACE("user out file ended");
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
            } else if (is_space_char(b)) {
                ungetc(a, fp_std);
                Na--;
            } else {
                FM_LOG_TRACE("WA ['%c':0x%x @%d] : ['%c':0x%x @%d]", a, a, Na, b, b, Nb);
                status = WA;
                break;
            }
        }
    }  // end of while

    fclose(fp_std);
    fclose(fp_exe);

    if (status == WA || status == PE) {
        make_diff_out2(file_out, file_user, oj_solution.work_dir, file_out);
    }
    FM_LOG_TRACE("compare finished, result=%s", status == AC ? "AC" : (status == PE ? "PE" : "WA"));
    return status;
}

void fix_java_result(const char *stdout_file, const char *stderr_file) {
    int comp_res = execute_cmd("/bin/grep -q 'java.lang.OutOfMemoryError' %s", stderr_file);
    if (!comp_res) {
        oj_solution.result = OJ_MLE;
        oj_solution.memory_usage = oj_solution.memory_limit * STD_KB;
        return;
    }

    comp_res = execute_cmd("/bin/grep -q 'java.lang.OutOfMemoryError' %s", stdout_file);
    if (!comp_res) {
        oj_solution.result = OJ_MLE;
        oj_solution.memory_usage = oj_solution.memory_limit * STD_KB;
        return;
    }

    comp_res = execute_cmd("/bin/grep -q 'Exception' %s", stderr_file);
    if (!comp_res) {
        oj_solution.result = OJ_RE;
        return;
    }

    comp_res = execute_cmd("/bin/grep -q 'Could not create' %s", stderr_file);
    if (!comp_res) {
        oj_solution.result = OJ_RE;
        return;
    }
}

int fix_gcc_result(const char *stderr_compiler) {
    if (oj_solution.lang != LANG_C99 &&
        oj_solution.lang != LANG_C11 &&
        oj_solution.lang != LANG_CPP98 &&
        oj_solution.lang != LANG_CPP11 &&
        oj_solution.lang != LANG_CPP14 &&
        oj_solution.lang != LANG_CPP17) {
        return 0;
    }

    int comp_res = execute_cmd("/bin/grep -q 'compiler error' %s", stderr_compiler);
    if (!comp_res) {
        execute_cmd("/bin/sed -n -i '1p' stderr_compiler.txt");
        return 1;
    }

    return 0;
}

// Output result
void output_acm_result(int result, long time_usage, long memory_usage, int test) {
    FM_LOG_MONITOR("result(%d): %s, time: %d ms, memory: %d KB",
                   result, result_str[result], time_usage, memory_usage);
    // this is judge result for Web app
    printf("%d %ld %ld %d\n", result, time_usage, memory_usage, test);
    FILE *fp = fopen("result.txt", "w");
    fprintf(fp, "%d %ld %ld %d", result, time_usage, memory_usage, test);
    fclose(fp);
}

int get_num_of_test() {
    struct dirent **namelist;
    int num_of_test;
    num_of_test = scandir(oj_solution.data_dir, &namelist, data_filter, alphasort);
    if (num_of_test < 0) {
        FM_LOG_FATAL("scan data directory failed: %s", strerror(errno));
        exit(EXIT_PRE_JUDGE_DAA);
    }
    for (int i = 0; i < num_of_test; ++i) {
        free(namelist[i]);
    }
    free(namelist);
    return num_of_test;

}
