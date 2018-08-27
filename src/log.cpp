//
// Created by w703710691d on 18-8-24.
//
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <sys/time.h>
#include <cstdarg>
#include <unistd.h>
#include <sys/file.h>
#include "log.h"

char LOG_LEVEL_NOTE[][10] = {"FATAL", "WARNING", "MONITOR", "NOTICE", "TRACE", "DEBUG"};
FILE *log_fp = nullptr;
char *log_filename = nullptr;
bool log_opened = false;

#define LOG_BUFFER_SIZE 8192
char log_buffer[LOG_BUFFER_SIZE];
char log_extra_info[LOG_BUFFER_SIZE];

int log_open(const char *filename) {
    if (log_opened == 1) {
        fprintf(stderr, "logger: log already opened\n");
        return 0;
    }
    size_t len = strlen(filename);
    log_filename = (char *) malloc(sizeof(char) * len + 1);
    snprintf(log_filename, len + 1, "%s", filename);
    log_fp = fopen(log_filename, "a");
    if (log_fp == nullptr) {
        fprintf(stderr, "log_file: %s", log_filename);
        perror("cannot open log file");
        exit(1);
    }

    atexit(log_close);
    log_opened = true;
    log_extra_info[0] = 0;
    FM_LOG_TRACE("log_open");
    return 1;
}

void log_close() {
    if (log_opened) {
        char buf[256] = "log_close\n";
        if (!log_extra_info[0]) {
            strcat(buf, "--------------------------------------------------------------------------------\n");
        }
        FM_LOG_TRACE(buf);
        fclose(log_fp);
        free(log_filename);
        log_fp = nullptr;
        log_filename = nullptr;
        log_opened = false;
    }
}

void log_write(int level, const char *file,
               const int line, const char *fmt, ...) {
    if (LOG_LEVEL < level) return;
    if (log_opened == 0) {
        fprintf(stderr, "log_open not called yet!\n");
        exit(1);
    }
    char buffer[LOG_BUFFER_SIZE];
    char datetime[100];
    timeval tv{};
    time_t now = time(nullptr);

    strftime(buffer, 90, "%Y-%m-%d %H:%M:%S", localtime(&now));
    if (gettimeofday(&tv, nullptr) == 0) {
        snprintf(datetime, sizeof(datetime), "%s.%06ld", buffer, tv.tv_usec);
    }

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log_buffer, LOG_BUFFER_SIZE, fmt, ap);
    va_end(ap);

    size_t count = (size_t) snprintf(buffer, LOG_BUFFER_SIZE, "%s [%s] [#%d] [%s:%d]%s %s\n", LOG_LEVEL_NOTE[level],
                                     datetime, getpid(), file, line, log_extra_info, log_buffer);

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

