#ifndef __MISC_H__
#define __MISC_H__

#include <ctype.h>
#include "judge_core.h"

#define is_space_char(a) ((a == ' ') || (a == '\t') || (a == '\n'))

int max(int a, int b);
long file_size(const char *filename);
int isInFile(const char *filename);
int strincmp(const char *s1, const char *s2, int n);
int malarm(int which, int milliseconds);
void print_compiler(const char * options[]);
int execute_cmd(const char *format, ...);
void copy_shell_runtime(const char * work_dir);
void copy_python_runtime(const char * work_dir);
void clean_workdir(const char * work_dir);
void make_diff_out(FILE *f1, FILE *f2, int c1, int c2, const char *work_dir, const char *path);
bool is_valid_syscall(int lang, int syscall_id);


int max(int a, int b)
{
  return a > b ? a : b;
}

long file_size(const char *filename)
{
  struct stat st; 

  if (!stat(filename, &st)) {
    return st.st_size;
  }
  
  return 0; 
}

int isInFile(const char *filename)
{
  int len = strlen(filename);
  if (len <= 3 || strcmp(filename + len - 3, ".in") != 0) {
    return 0;
  }
  else {
    return len - 3;
  }
}

// copied from
// http://csourcesearch.net/c/fid471AEC75A44B4EB7F79BAB9F1C5DE7CA616177E5.aspx
int strincmp(const char *s1, const char *s2, int n)
{
  /* case insensitive comparison */
  int d;
  while (--n >= 0) {
#ifdef ASCII_CTYPE
    if (!isascii(*s1) || !isascii(*s2))
      d = *s1 - *s2;
    else
#endif
      d = (tolower((unsigned char)*s1) - tolower((unsigned char)*s2));
    if ( d != 0 || *s1 == '\0' || *s2 == '\0' )
      return d;
    ++s1;
    ++s2;
  }
  return 0;
}

//a simpler interface for setitimer
//which can be ITIMER_REAL, ITIMER_VIRTUAL, ITIMER_PROF
int malarm(int which, int milliseconds)
{
  struct itimerval t;
  FM_LOG_TRACE("malarm: %d", milliseconds);
  t.it_value.tv_sec       = milliseconds / 1000;
  t.it_value.tv_usec      = milliseconds % 1000 * 1000;
  t.it_interval.tv_sec    = 0;
  t.it_interval.tv_usec   = 0;
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

void copy_shell_runtime(const char *work_dir)
{
  execute_cmd("/bin/mkdir %s/lib", work_dir);
  execute_cmd("/bin/mkdir %s/bin", work_dir);
  execute_cmd("/bin/cp /lib/* %s/lib/", work_dir);
  execute_cmd("/bin/cp -a /lib/i386-linux-gnu %s/lib/", work_dir);
#ifndef __i386
  execute_cmd("/bin/mkdir %s/lib64", work_dir);
  execute_cmd("/bin/cp -a /lib/x86_64-linux-gnu %s/lib/", work_dir);
  execute_cmd("/bin/cp /lib64/* %s/lib64/", work_dir);
  execute_cmd("/bin/cp -a /lib32 %s/", work_dir);
#endif
  execute_cmd("/bin/cp /bin/busybox %s/bin/", work_dir);
  execute_cmd("/bin/ln -s /bin/busybox %s/bin/sh", work_dir);
  execute_cmd("/bin/cp /bin/bash %s/bin/bash", work_dir);
}

void copy_python_runtime(const char *work_dir)
{
  copy_shell_runtime(work_dir);
  execute_cmd("/bin/mkdir -p %s/usr/include", work_dir);
  execute_cmd("/bin/mkdir -p %s/usr/lib", work_dir);
  execute_cmd("/bin/cp /usr/bin/python* %s/", work_dir);
  execute_cmd("/bin/cp -a /usr/lib/python* %s/usr/lib/", work_dir);
  execute_cmd("/bin/cp -a /usr/include/python* %s/usr/include/", work_dir);
  execute_cmd("/bin/cp -a /usr/lib/libpython* %s/usr/lib/", work_dir);
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

// not very helpful, should diff line by line and give line number
void make_diff_out(FILE *f1, FILE *f2, int c1, int c2, const char *work_dir, const char *path)
{
  FM_LOG_DEBUG("make_diff_out");
  FILE *out;
  char buf[BUFF_SIZE];
  sprintf(buf, "%s/diff.out", work_dir);
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

#include "syscalls.h"
// deal with sys call
static bool in_syscall = true;
bool is_valid_syscall(int lang, int syscall_id)
{
  in_syscall = !in_syscall;
  if (syscalls[syscall_id] == 0) {
    return false;
  }
  else if (syscalls[syscall_id] > 0) {
    if (in_syscall == false) {
      syscalls[syscall_id]--;
    }
  }
  else {
    ;
  }
  return true;
}

#endif /* __MISC_H__ */