/*
 * Copyright 2014 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
#ifndef SRC_MISC_H_
#define SRC_MISC_H_

#include "judge_core.h"


#define is_space_char(a) ((a == ' ') || (a == '\t') || (a == '\n'))

int max(int a, int b);
off_t file_size(const char *filename);
int checkInFile(const char *filename);
int malarm(int which, int milliseconds);
void print_compiler(const char * options[]);
int execute_cmd(const char *format, ...);
void make_diff_out(FILE *f1, FILE *f2, int c1, int c2, const char *work_dir, const char *path);
#ifndef FAST_JUDGE
void copy_shell_runtime(const char * work_dir);
void copy_python_runtime(const char * work_dir);
void clean_workdir(const char * work_dir);
#endif


int max(int a, int b)
{
  return a > b ? a : b;
}

off_t file_size(const char *filename)
{
  struct stat st;

  if (!stat(filename, &st)) {
    return st.st_size;
  }

  return 0;
}

int checkInFile(const char *filename)
{
  int len = strlen(filename);
  if (len <= 3 || strcmp(filename + len - 3, ".in") != 0) {
    return 0;
  } else {
    return len - 3;
  }
}

// a simpler interface for setitimer
// which can be ITIMER_REAL, ITIMER_VIRTUAL, ITIMER_PROF
int malarm(int which, int milliseconds)
{
  struct itimerval t;
  t.it_value.tv_sec       = milliseconds / 1000;
  t.it_value.tv_usec      = milliseconds % 1000 * 1000;
  t.it_interval.tv_sec    = 0;
  t.it_interval.tv_usec   = 0;
  FM_LOG_TRACE("malarm: %d", milliseconds);
  return setitimer(which, &t, NULL);
}

void print_compiler(const char *options[])
{
  int i = 0;
  char buff[BUFF_SIZE] = {0};
  while (options[i] != NULL) {
    strcat(buff, options[i++]);
    strcat(buff, " ");
  }
  FM_LOG_DEBUG(buff);
}

int execute_cmd(const char *fmt, ...)
{
  char cmd[BUFF_SIZE];
  va_list ap;

  va_start(ap, fmt);
  vsprintf(cmd, fmt, ap);
  int ret = system(cmd);
  va_end(ap);
  return ret;
}

#ifndef FAST_JUDGE
void copy_shell_runtime(const char *work_dir)
{
  execute_cmd("/bin/mkdir %s/lib 2>>error.log", work_dir);
  execute_cmd("/bin/mkdir %s/bin 2>>error.log", work_dir);
  execute_cmd("/bin/cp /lib/* %s/lib/ 2>>error.log", work_dir);
  execute_cmd("/bin/cp -a /lib/i386-linux-gnu %s/lib/ 2>>error.log", work_dir);
#ifndef __i386
  execute_cmd("/bin/mkdir %s/lib64 2>>error.log", work_dir);
  execute_cmd("/bin/cp -a /lib/x86_64-linux-gnu %s/lib/ 2>>error.log", work_dir);
  execute_cmd("/bin/cp /lib64/* %s/lib64/ 2>>error.log", work_dir);
  execute_cmd("/bin/cp -a /lib32 %s/ 2>>error.log", work_dir);
#endif
  execute_cmd("/bin/cp /bin/busybox %s/bin/ 2>>error.log", work_dir);
  execute_cmd("/bin/cp /bin/bash %s/bin/bash 2>>error.log", work_dir);
}

void copy_python_runtime(const char *work_dir)
{
  copy_shell_runtime(work_dir);
  execute_cmd("/bin/mkdir -p %s/usr/include 2>>error.log", work_dir);
  execute_cmd("/bin/mkdir -p %s/usr/lib 2>>error.log", work_dir);
  execute_cmd("/bin/cp /usr/bin/python* %s/ 2>>error.log", work_dir);
  execute_cmd("/bin/cp -a /usr/lib/python2* %s/usr/lib/ 2>>error.log", work_dir);
  execute_cmd("/bin/cp -a /usr/lib/libpython2* %s/usr/lib/ 2>>error.log", work_dir);
  execute_cmd("/bin/cp -a /usr/include/python2* %s/usr/include/ 2>>error.log", work_dir);
}

void clean_workdir(const char *work_dir)
{
  execute_cmd("rm -Rf %s/lib", work_dir);
  execute_cmd("rm -Rf %s/lib32", work_dir);
  execute_cmd("rm -Rf %s/lib64", work_dir);
  execute_cmd("rm -Rf %s/bin", work_dir);
  execute_cmd("rm -Rf %s/usr", work_dir);
  execute_cmd("rm -f %s/python*", work_dir);
}
#endif

// not very helpful, should diff line by line and give line number
void make_diff_out(FILE *f1, FILE *f2, int c1, int c2, const char *work_dir, const char *path)
{
  FM_LOG_DEBUG("make_diff_out");
  FILE *out;
  char buf[BUFF_SIZE];
  snprintf(buf, BUFF_SIZE, "%s/diff.out", work_dir);
  out = fopen(buf, "a+");
  fprintf(out, "=================%s\n", basename(path));
  fprintf(out, "Right:\n%c", c1);
  if (fgets(buf, 80, f1)) {
    fprintf(out, "%s", buf);
  }
  fprintf(out, "\n-----------------\n");
  fprintf(out, "Your:\n%c", c2);
  if (fgets(buf, 80, f2)) {
    fprintf(out, "%s", buf);
  }
  fprintf(out, "\n=================\n");
  fclose(out);
}

#endif  // SRC_MISC_H_
