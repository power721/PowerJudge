//
// Created by w703710691d on 18-8-24.
//

#ifndef POWERJUDGE_READ_CONFIG_H
#define POWERJUDGE_READ_CONFIG_H

#include <cstdio>
#include "judge_core.h"

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
};
extern const char *DEFAULT_CFG_FILE;
extern oj_config_t oj_config;
size_t split(char *line, char **key, char **value);

void read_config(const char *cfg_file);

#endif //POWERJUDGE_READ_CONFIG_H
