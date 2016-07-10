/*
 * Copyright 2015 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
#ifndef SRC_JUDGED_H_
#define SRC_JUDGED_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bsd/libutil.h>
#include <curl/curl.h>

#include "log.h"
#include "misc.h"

#define ERROR_READ_FILE 40
#define ERROR_READ_RESULT 41

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
}oj_config;

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
}oj_solution;

int DEFAULT_BACKLOG = 100;
uint16_t DEFAULT_PORT = 55555;
int MAX_UPLOAD_FILE_SIZE = 4096;
const char *DEFAULT_CFG_FILE = "/home/judge/judge.properties";
const char *PID_FILE = "/var/run/judged.pid";
const char *LOG_FILE = "/var/log/judged.log";


void signal_handler(int signo);
void work(int newsockfd, struct sockaddr_in cli_addr);
void check_pid();
void read_config(const char *cfg_file);
size_t split(char *line, char **key, char **value);
int check_password(char *password, char *message);
int parse_arguments(char *str);
void run();
void update_result();
void update_system_error(int result);
void send_multi_result(char* file_path);
void truncate_upload_file(char* file_path);

#endif  // SRC_JUDGED_H_
