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

#include "log.h"
#include "misc.h"

struct oj_config_t {
    char ip[20];
    int port;
    int backlog;

    char password[256];
    char data_dir[PATH_SIZE];
    char temp_dir[PATH_SIZE];
}oj_config;

struct oj_solution_t {
  char sid[15];           // solution id
  char pid[15];           // problem id
  char language[5];       // language id
  char time_limit[15];    // ms
  char memory_limit[15];  // KB
}oj_solution;

int DEFAULT_BACKLOG = 100;
int DEFAULT_PORT = 55555;
const char *DEFAULT_CFG_FILE = "/home/judge/judge.properties";
const char *PID_FILE = "/var/run/judged.pid";
const char *LOG_FILE = "/var/log/judged.log";


void work(int newsockfd, struct sockaddr_in cli_addr);
void check_pid();
void read_config(const char *cfg_file);
int split(char *line, char **key, char **value);
int check_password(char *password, char *message);
int parse_arguments(char *str);
void run();

#endif  // SRC_JUDGED_H_
