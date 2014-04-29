#ifndef __HELPER_H__
#define __HELPER_H__

#include "judge_core.h"

int malarm(int which, int milliseconds);
long files_ize(const char *filename);
int execute_cmd(const char *format, ...);
void print_compiler(const char * options[]);

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

void print_compiler(const char * options[])
{
  int i = 0;
  char buff[BUFF_SIZE] = {0};
  while (options[i] != NULL)
  {
    sprintf(buff, "%s %s", buff, options[i]);
    i++;
  }
  FM_LOG_TRACE(buff);
}

#endif