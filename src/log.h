/*
 * Copyright 2014 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
/*
*
* LOGGER v0.0.4
* modified by power(power0721#gmail.com)
* A simple logger for c/c++ under linux, multiprocess-safe
*
* ---- CopyLeft by Felix021 @ http://www.felix021.com ----
*
* LOG Format:
* --LEVEL_NOTE--\x7 [Y-m-d H:m:s]\x7 [FILE:LINE]\x7 [EXTRA_INFO]\x7 log_info
*   // LEVEL_NOTE stands for one of DEBUG/TRACE/NOTICE...
*   // \x7 is a special character to separate logged fields.
*
* Usage:
*   //Open log file first. Supply a log file name.
*   log_open("log.txt");
*
*   //use it just as printf
*   FM_LOG_TRACE("some info %d", 123);
*
*   //6 level: DEBUG, TRACE, NOTICE, MONITOR, WARNING, FATAL
*   FM_LOG_DEBUG("hi there");
*
*   //Need EXTRA_INFO to be logged automatically?
*   log_add_info("pid:123");
*
*   //You don't need to call log_close manually, it'll be called at exit
*   log_close();
*
*/
#ifndef SRC_LOG_H_
#define SRC_LOG_H_


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <error.h>
#include <sys/file.h>
#include <sys/time.h>


int log_open(const char *filename);

void log_close();

static void log_write(int, const char *, const int, const char *, ...);

void log_add_info(const char *info);


#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

#define LOG_FATAL        0
#define LOG_WARNING      1
#define LOG_MONITOR      2
#define LOG_NOTICE       3
#define LOG_TRACE        4
#define LOG_DEBUG        5
static char LOG_LEVEL_NOTE[][10] =
        {"FATAL", "WARNING", "MONITOR", "NOTICE", "TRACE", "DEBUG"};

#define FM_LOG_DEBUG(x...)   log_write(LOG_DEBUG,   __FILE__, __LINE__, ##x)
#define FM_LOG_TRACE(x...)   log_write(LOG_TRACE,   __FILE__, __LINE__, ##x)
#define FM_LOG_NOTICE(x...)  log_write(LOG_NOTICE,  __FILE__, __LINE__, ##x)
#define FM_LOG_MONITOR(x...) log_write(LOG_MONITOR, __FILE__, __LINE__, ##x)
#define FM_LOG_WARNING(x...) log_write(LOG_WARNING, __FILE__, __LINE__, ##x)
#define FM_LOG_FATAL(x...)   log_write(LOG_FATAL,   __FILE__, __LINE__, ##x)

static FILE *log_fp = NULL;
static char *log_filename = NULL;
static int log_opened = 0;

#define LOG_BUFFER_SIZE 8192
static char log_buffer[LOG_BUFFER_SIZE];
static char log_extra_info[LOG_BUFFER_SIZE];


int log_open(const char *filename) {
  if (log_opened == 1) {
    fprintf(stderr, "logger: log already opened\n");
    return 0;
  }
  size_t len = strlen(filename);
  log_filename = (char *) malloc(sizeof(char) * len + 1);
  snprintf(log_filename, len + 1, "%s", filename);
  log_fp = fopen(log_filename, "a");
  if (log_fp == NULL) {
    fprintf(stderr, "log_file: %s", log_filename);
    perror("cannot open log file");
    exit(1);
  }

  atexit(log_close);
  log_opened = 1;
  log_extra_info[0] = 0;
  FM_LOG_TRACE("log_open");
  return 1;
}


void log_close(void) {
  if (log_opened) {
    char buf[256] = "log_close\n";
    if (!log_extra_info[0]) {
      strcat(buf,
             "--------------------------------------------------------------------------------\n");
    }
    FM_LOG_TRACE(buf);
    fclose(log_fp);
    free(log_filename);
    log_fp = NULL;
    log_filename = NULL;
    log_opened = 0;
  }
}

static void log_write(int level, const char *file,
                      const int line, const char *fmt, ...) {
  if (LOG_LEVEL < level) return;
  if (log_opened == 0) {
    fprintf(stderr, "log_open not called yet!\n");
    exit(1);
  }
  static char buffer[LOG_BUFFER_SIZE];
  static char datetime[100];
  static time_t now;
  struct timeval tv;
  now = time(NULL);

  strftime(buffer, 90, "%Y-%m-%d %H:%M:%S", localtime(&now));
  if (gettimeofday(&tv, NULL) == 0) {
    snprintf(datetime, sizeof(datetime), "%s.%06ld", buffer, tv.tv_usec);
  }

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(log_buffer, LOG_BUFFER_SIZE, fmt, ap);
  va_end(ap);

  size_t count = snprintf(buffer, LOG_BUFFER_SIZE,
                          "%s [%s] [#%d] [%s:%d]%s %s\n",
                          LOG_LEVEL_NOTE[level], datetime, getpid(),
                          file, line, log_extra_info, log_buffer);

  if (level == LOG_FATAL) {
    fprintf(stderr, "%s\n", log_buffer);
  }

  int log_fd = log_fp->_fileno;
  if (flock(log_fd, LOCK_EX) == 0) {
    if (write(log_fd, buffer, count) < 0) {
      perror("write log error");
      exit(1);
    }
    flock(log_fd, LOCK_UN);
  } else {
    perror("flock log file error");
    exit(1);
  }
}

void log_add_info(const char *info) {
  size_t len = strlen(log_extra_info);
  snprintf(log_extra_info + len, LOG_BUFFER_SIZE - len, " [%s]", info);
}

#endif  // SRC_LOG_H_
