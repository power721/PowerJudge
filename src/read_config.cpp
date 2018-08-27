//
// Created by w703710691d on 18-8-24.
//
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include "read_config.h"
#include "misc.h"
#include "log.h"
#include "judge_core.h"

const char *DEFAULT_CFG_FILE = "/etc/judged.conf";
uint16_t DEFAULT_PORT = 55555;
int MAX_UPLOAD_FILE_SIZE = 4096;
uint16_t DEFAULT_THREAD_NUM = 1;
int DEFAULT_BACKLOG = 100;
oj_config_t oj_config;

size_t split(char *line, char **key, char **value) {
    int index = 0;
    size_t val_len = 0;
    for (char *p = line; *p; p++, index++) {
        if (*p == '=') {
            line[index] = '\0';
            *key = trim(line);
            *value = trim(line + index + 1);
            val_len = strlen(*value);
            break;
        }
    }
    return val_len;
}

void read_config(const char *cfg_file) {
    FM_LOG_NOTICE("config file: %s", cfg_file);
    FILE *fp = fopen(cfg_file, "r");
    char *line = nullptr;
    size_t len = 0;
    ssize_t read;

    if (fp == nullptr) {
        fatal_error("cannot open configuration file");
    }

    char *key, *value;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (read > 0) {
            size_t val_len = split(line, &key, &value);
            if (val_len == 0) {
                continue;
            }

            if (strcmp("ip", key) == 0) {
                strncpy(oj_config.ip, value, val_len);
            } else if (strcmp("port", key) == 0) {
                oj_config.port = (uint16_t) atoi(value);
            } else if (strcmp("thread", key) == 0) {
                oj_config.thread_num = (uint16_t) atoi(value);
            } else if (strcmp("backlog", key) == 0) {
                oj_config.backlog = atoi(value);
            } else if (strcmp("password", key) == 0) {
                strncpy(oj_config.password, value, val_len);
            } else if (strcmp("data.dir", key) == 0) {
                strncpy(oj_config.data_dir, value, val_len);
            } else if (strcmp("temp.dir", key) == 0) {
                strncpy(oj_config.temp_dir, value, val_len);
            } else if (strcmp("db.host", key) == 0) {
                strncpy(oj_config.db_host, value, val_len);
            } else if (strcmp("db.port", key) == 0) {
                oj_config.db_port = (uint16_t) atoi(value);
            } else if (strcmp("db.user", key) == 0) {
                strncpy(oj_config.db_user, value, val_len);
            } else if (strcmp("db.password", key) == 0) {
                strncpy(oj_config.db_password, value, val_len);
            } else if (strcmp("db.database", key) == 0) {
                strncpy(oj_config.db_database, value, val_len);
            } else if (strcmp("api.url", key) == 0) {
                strncpy(oj_config.api_url, value, val_len);
            } else if (strcmp("user.agent", key) == 0) {
                strncpy(oj_config.user_agent, value, val_len);
            }
        }
    }

    if (oj_config.port <= 0) {
        oj_config.port = DEFAULT_PORT;
    }
    if (oj_config.backlog <= 0) {
        oj_config.backlog = DEFAULT_BACKLOG;
    }
    if (oj_config.thread_num <= 0) {
        oj_config.thread_num = DEFAULT_THREAD_NUM;
    }

    fclose(fp);
    if (line) {
        free(line);
    }
}
