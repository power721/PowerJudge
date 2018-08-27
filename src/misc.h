//
// Created by w703710691d on 18-8-24.
//
/*
 * Copyright 2014 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
#ifndef POWERJUDGE_MISC_H
#define POWERJUDGE_MISC_H

#include <cstdio>
#include <cstdint>

#define is_space_char(a) ((a == ' ') || (a == '\t') || (a == '\n'))

#endif //POWERJUDGE_MISC_H

void fatal_error(const char *msg);

char *trim(char *str);

off_t file_size(const char *filename);

size_t checkInFile(const char *filename);

int malarm(int which, uint64_t milliseconds);

void print_compiler(const char *options[]);

void print_executor(const char *options[]);

void print_user_group();

void print_word_dir();

int execute_cmd(const char *format, ...);

void make_diff_out(FILE *f1, FILE *f2, int c1, int c2, const char *work_dir, const char *path);

void make_diff_out2(const char *file_out, const char *file_user, const char *work_dir, const char *path);

void check_and_rename_log(const char *filename);

#ifndef FAST_JUDGE

void copy_shell_runtime(const char *work_dir);

void copy_python_runtime_python2(const char *work_dir);

void copy_python_runtime_python3(const char *work_dir);

void clean_workdir(const char *work_dir);

#endif
