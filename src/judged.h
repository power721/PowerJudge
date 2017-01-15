/*		
 * Copyright 2015 power <power0721#gmail.com>		
 * PowerOJ GPLv2		
 * modify to run multi-thread 2016 w703710691d <w703710691d#163.com>
 */
#ifndef SRC_JUDGED_H_
#define SRC_JUDGED_H_

#include<bits/stdc++.h>

#define OJ_AC       0
#define OJ_PE       1
#define OJ_TLE      2
#define OJ_MLE      3
#define OJ_WA       4
#define OJ_RE       5
#define OJ_OLE      6
#define OJ_CE       7
#define OJ_RF       8
#define OJ_SE       9
#define OJ_VE       10
#define OJ_WAIT     11
#define OJ_RUN      12

#define EXIT_OK                0
#define EXIT_UNPRIVILEGED      1
#define EXIT_CHDIR             2
#define EXIT_BAD_PARAM         3
#define EXIT_MISS_PARAM        4
#define EXIT_VERY_FIRST        5
#define EXIT_FORK_COMPILER     6
#define EXIT_COMPILE_IO        6
#define EXIT_COMPILE_EXEC      6
#define EXIT_COMPILE_ERROR     6
#define EXIT_NO_SOURCE_CODE    7
#define EXIT_PRIVILEGED        8
#define EXIT_PRE_JUDGE         9
#define EXIT_PRE_JUDGE_PTRACE  10
#define EXIT_PRE_JUDGE_EXECLP  11
#define EXIT_PRE_JUDGE_DAA     12
#define EXIT_FORK_ERROR        13
#define EXIT_EXEC_ERROR        14
#define EXIT_SET_LIMIT         15
#define EXIT_CURL_ERROR        16
#define EXIT_SET_SECURITY      17
#define EXIT_JUDGE             21
#define EXIT_COMPARE           27
#define EXIT_COMPARE_SPJ       30
#define EXIT_COMPARE_SPJ_FORK  31
#define EXIT_TIMEOUT           36
#define EXIT_UNKNOWN           127

#define ERROR_READ_FILE 40
#define ERROR_READ_RESULT 41
#define PATH_SIZE 4096
#define BUFF_SIZE 8192

struct oj_config_t {
    char ip[20];
    uint16_t port;
    int backlog;

    char password[256];
    char data_dir[PATH_SIZE];
    char temp_dir[PATH_SIZE];

    char db_host[256];
    uint16_t db_port;
    char db_user[256];
    char db_password[256];
    char db_database[256];
    char api_url[256];
    char user_agent[256];
    uint16_t thread_num;
} oj_config;

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

int DEFAULT_BACKLOG = 100;
uint16_t DEFAULT_PORT = 55555;
int MAX_UPLOAD_FILE_SIZE = 4096;
uint16_t DEFAULT_THREAD_NUM = 1;
const char *DEFAULT_CFG_FILE = "/etc/judged.conf";
const char *PID_FILE = "/var/run/judged.pid";
const char *LOG_FILE = "/var/log/judged.log";


void signal_handler(int signo);

void work(int newsockfd, struct sockaddr_in cli_addr);

void check_pid();

void read_config(const char *cfg_file);

size_t split(char *line, char **key, char **value);

int check_password(char *password, char *message);

int parse_arguments(char *str, oj_solution_t &oj_solution);

void run(oj_solution_t &oj_solution);

void update_result(oj_solution_t &oj_solution);

void update_system_error(int result, oj_solution_t &oj_solution);

void send_multi_result(char *file_path, oj_solution_t &oj_solution);

void truncate_upload_file(char *file_path);

void fatal_error(const char *msg);

void print_user_group();

void print_word_dir();

char *trim(char *str);

off_t file_size(const char *filename);

#endif  // SRC_JUDGED_H_
