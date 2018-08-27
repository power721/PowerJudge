//
// Created by w703710691d on 18-8-24.
//

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include "misc.h"
#include "log.h"
#include "judge_core.h"
#include <sys/time.h>
#include <cstdarg>
#include "errno.h"

void fatal_error(const char *msg) {
    perror(msg);
    exit(1);
}

char *trim(char *str) {
    char *end;

    // Trim leading space
    while (isspace(*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

off_t file_size(const char *filename) {
    struct stat st{};

    if (!stat(filename, &st)) {
        return st.st_size;
    }

    return 0;
}

size_t checkInFile(const char *filename) {
    size_t len = strlen(filename);
    if (len <= 3 || strcmp(filename + len - 3, ".in") != 0) {
        return 0;
    } else {
        return len - 3;
    }
}

// a simpler interface for setitimer
// which can be ITIMER_REAL, ITIMER_VIRTUAL, ITIMER_PROF
int malarm(int which, uint64_t milliseconds) {
    itimerval t{};
    t.it_value.tv_sec = milliseconds / 1000;
    t.it_value.tv_usec = milliseconds % 1000 * 1000;
    t.it_interval.tv_sec = 0;
    t.it_interval.tv_usec = 0;
    FM_LOG_TRACE("malarm: %d ms", milliseconds);
    return setitimer(which, &t, nullptr);
}

void print_word_dir() {
    char cwd[PATH_SIZE];
    char *tmp = getcwd(cwd, PATH_SIZE - 1);
    if (tmp == nullptr) {
        FM_LOG_WARNING("getcwd failed: %s", strerror(errno));
    } else {
        FM_LOG_NOTICE(cwd);
    }
}

void print_user_group() {
    uid_t ruid, euid, suid;
    gid_t rgid, egid, sgid;
    getresuid(&ruid, &euid, &suid);
    getresgid(&rgid, &egid, &sgid);
    FM_LOG_DEBUG("ruid=%d euid=%d suid=%d rgid=%d egid=%d sgid=%d", ruid, euid, suid, rgid, egid, sgid);
}

void print_compiler(const char *options[]) {
    int i = 0;
    char buff[BUFF_SIZE] = {0};
    while (options[i] != nullptr) {
        strcat(buff, options[i++]);
        strcat(buff, " ");
    }
    FM_LOG_DEBUG(buff);
}

void print_executor(const char *options[]) {
    int i = 0;
    char buff[BUFF_SIZE] = {0};
    while (options[i] != nullptr) {
        strcat(buff, options[i++]);
        strcat(buff, " ");
    }
    FM_LOG_DEBUG(buff);
}

int execute_cmd(const char *fmt, ...) {
    char cmd[BUFF_SIZE];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    int ret = system(cmd);
    va_end(ap);
    return ret;
}

#ifndef FAST_JUDGE

void copy_shell_runtime(const char *work_dir) {
    execute_cmd("/bin/mkdir %s/lib 2>>error.log", work_dir);
    execute_cmd("/bin/mkdir %s/bin 2>>error.log", work_dir);
    execute_cmd("/bin/cp /lib/* %s/lib/ 2>>error.log", work_dir);
#ifndef __i386
    execute_cmd("/bin/mkdir %s/lib64 2>>error.log", work_dir);
    execute_cmd("/bin/cp -a /lib/x86_64-linux-gnu %s/lib/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp /lib64/* %s/lib64/ 2>>error.log", work_dir);
#else
    execute_cmd("/bin/cp -a /lib/i386-linux-gnu %s/lib/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp -a /lib32 %s/ 2>>error.log", work_dir);
#endif
    execute_cmd("/bin/cp /bin/busybox %s/bin/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp /bin/bash %s/bin/bash 2>>error.log", work_dir);
}

void copy_python_runtime_python2(const char *work_dir) {
    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/include 2>>error.log", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/lib 2>>error.log", work_dir);
    execute_cmd("/bin/cp /usr/bin/python %s/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp /usr/bin/python2* %s/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp -a /usr/lib/python2* %s/usr/lib/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp -a /usr/lib/libpython2* %s/usr/lib/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp -a /usr/include/python2* %s/usr/include/ 2>>error.log", work_dir);
}

void copy_python_runtime_python3(const char *work_dir) {
    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/include 2>>error.log", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/lib 2>>error.log", work_dir);
    execute_cmd("/bin/cp /usr/bin/python %s/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp /usr/bin/python3* %s/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp -a /usr/lib/python3* %s/usr/lib/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp -a /usr/lib/libpython3* %s/usr/lib/ 2>>error.log", work_dir);
    execute_cmd("/bin/cp -a /usr/include/python3* %s/usr/include/ 2>>error.log", work_dir);
}

void clean_workdir(const char *work_dir) {
    execute_cmd("rm -Rf %s/lib", work_dir);
#ifdef __i386
    execute_cmd("rm -Rf %s/lib32", work_dir);
#else
    execute_cmd("rm -Rf %s/lib64", work_dir);
#endif
    execute_cmd("rm -Rf %s/bin", work_dir);
    execute_cmd("rm -Rf %s/usr", work_dir);
    execute_cmd("rm -f %s/python*", work_dir);
}

#endif

// not very helpful, should diff line by line and give line number
void make_diff_out(FILE *f1, FILE *f2, int c1, int c2, const char *work_dir, const char *path) {
    FM_LOG_DEBUG("make_diff_out");
    FILE *out;
    char buf[BUFF_SIZE];
    snprintf(buf, BUFF_SIZE, "%s/diff.out", work_dir);
    out = fopen(buf, "a+");
    fprintf(out, "=================%s\n", basename((char *) path));
    fprintf(out, "Right:\n%c", c1);
    if (fgets(buf, BUFF_SIZE, f1)) {
        fprintf(out, "%s", buf);
    }
    fprintf(out, "\n-----------------\n");
    fprintf(out, "Your:\n%c", c2);
    if (fgets(buf, BUFF_SIZE, f2)) {
        fprintf(out, "%s", buf);
    }
    fprintf(out, "\n=================\n");
    fclose(out);
}

void make_diff_out2(const char *file_out, const char *file_user, const char *work_dir,
                    const char *path) {
    FM_LOG_DEBUG("make_diff_out2");
    FILE *fp_std = fopen(file_out, "r");
    if (fp_std == nullptr) {
        FM_LOG_WARNING("open standard output file (%s) failed: %s", file_out, strerror(errno));
        return;
    }

    FILE *fp_exe = fopen(file_user, "r");
    if (fp_exe == nullptr) {
        FM_LOG_WARNING("open user output file (%s) failed: %s", file_user, strerror(errno));
        return;
    }

    FILE *out;
    bool findDiff = false;
    int line = 0;
    char buf[BUFF_SIZE];
    char line1[BUFF_SIZE];
    char line2[BUFF_SIZE];
    snprintf(buf, BUFF_SIZE, "%s/diff.out", work_dir);
    out = fopen(buf, "a+");
    fprintf(out, "=================%s\n", basename((char *) path));
    while (fgets(line1, BUFF_SIZE, fp_std) != nullptr) {
        line++;
        if (fgets(line2, BUFF_SIZE, fp_exe) != nullptr) {
            if (strcmp(line1, line2) != 0) {
                findDiff = true;
                break;
            }
        } else {
            strncpy(line2, "<EOF>", 5);
            findDiff = true;
            break;
        }
    }

    if (!findDiff) {
        if (fgets(line2, BUFF_SIZE, fp_exe) != nullptr) {
            strncpy(line1, "<EOF>", 5);
        }
    }

    size_t n = strlen(line1);
    if (line1[n - 1] == '\n') {
        line1[n - 1] = '\0';
    }
    n = strlen(line2);
    if (line2[n - 1] == '\n') {
        line2[n - 1] = '\0';
    }
    fprintf(out, "Expect(Line #%d):\n%s", line, line1);
    fprintf(out, "\n-----------------\n");
    fprintf(out, "Your(Line #%d):\n%s", line, line2);
    fprintf(out, "\n=================\n");

    fclose(fp_std);
    fclose(fp_exe);
    fclose(out);
}

void check_and_rename_log(const char *filename) {
    off_t fsize = file_size(filename);
    if (fsize < MAX_LOG_FILE_SIZE) {
        return;
    }

    char backup_filename[PATH_SIZE];
    snprintf(backup_filename, PATH_SIZE, "%s.1", filename);

    rename(filename, backup_filename);
}

