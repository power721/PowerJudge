//
// Created by w703710691d on 18-8-27.
//

#ifndef POWERJUDGE_JUDGED_H
#define POWERJUDGE_JUDGED_H

#include "judge_core.h"

const char *PID_FILE = "/var/run/judged.pid";
const char *LOG_FILE = "/var/log/judged.log";
const int MAX_UPLOAD_FILE_SIZE = 4096;
struct oj_solution_t {
    char sid[15];           // solution id
    char pid[15];           // problem id
    int cid;                // contest id
    char language[5];       // language id
    char time_limit[15];    // ms
    char memory_limit[15];  // KB
    char token[255];
    char work_dir[PATH_SIZE];
    int result;
    int time_usage;
    int memory_usage;
    int test;
};

void run(oj_solution_t &oj_solution);

void update_result(oj_solution_t &oj_solution);

void update_system_error(int result, oj_solution_t &oj_solution);

void check_pid();

void signal_handler(int signo);

void work(int newsockfd, struct sockaddr_in cli_addr);

int check_password(char *password, char *message);

int parse_arguments(char *str, oj_solution_t &oj_solution);

void send_multi_result(char *file_path, oj_solution_t &oj_solution);

void truncate_upload_file(char *file_path);

#endif //POWERJUDGE_JUDGED_H
