/*
 * Copyright 2015 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
#include "judged.h"

struct pidfh *pfh;

int main(int argc, char *argv[], char *envp[])
{
    log_open(LOG_FILE);
    FM_LOG_TRACE("---");
    check_pid();

    int sockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    const char *cfg_file;

    if (argc > 1) {
        cfg_file = argv[1];
    } else {
        cfg_file = DEFAULT_CFG_FILE;
    }
    read_config(cfg_file);
    FM_LOG_TRACE("read_config");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    FM_LOG_TRACE("socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    if (inet_aton(oj_config.ip, &serv_addr.sin_addr) == 0) {
        serv_addr.sin_addr.s_addr = INADDR_ANY;
    }
    serv_addr.sin_port = htons(oj_config.port);
    FM_LOG_NOTICE("IP address: %s %s", oj_config.ip, inet_ntoa(serv_addr.sin_addr));
    FM_LOG_NOTICE("port: %d %d", oj_config.port, ntohs(serv_addr.sin_port));

    int yes = 1;
    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
    {
        error("setsockopt SO_REUSEADDR failed");
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }
    FM_LOG_TRACE("bind");

    clilen = sizeof(cli_addr);

    if (listen(sockfd, oj_config.backlog) < 0) {
        error("ERROR on listening");
    }
    FM_LOG_NOTICE("listen  backlog: %d", oj_config.backlog);

    if (daemon(0, 0) == -1) {
        FM_LOG_FATAL("Cannot daemonize");
        pidfile_remove(pfh);

        exit(EXIT_FAILURE);
    }

    pidfile_write(pfh);

    while (true) {
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        work(newsockfd, cli_addr);
    }

    pidfile_remove(pfh);
    close(sockfd);
    return 0;
}

void work(int newsockfd, struct sockaddr_in cli_addr) {
    if (newsockfd < 0) {
      FM_LOG_WARNING("ERROR on accept");
      return;
    }
    FM_LOG_NOTICE("connect from %s:%d", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

    char buffer[BUFF_SIZE];
    bzero(buffer, BUFF_SIZE);
    int n = read(newsockfd, buffer, BUFF_SIZE);
    if (n < 0) FM_LOG_WARNING("ERROR reading from socket");

    FM_LOG_NOTICE("Here is the password: %s", buffer);
    if (check_password(oj_config.password, buffer)) {
        n = write(newsockfd, "Authentication Ok", 17);
        if (n < 0) FM_LOG_WARNING("ERROR writing to socket");

        bzero(buffer, BUFF_SIZE);
        int n = read(newsockfd, buffer, BUFF_SIZE);
        if (n < 0) {
            FM_LOG_WARNING("ERROR reading from socket");
            close(newsockfd);
            return;
        }
        FM_LOG_NOTICE("Here is the message: %s", buffer);

        if (parse_arguments(buffer) < 0) {
            n = write(newsockfd, "Missing some parameters", 23);
            if (n < 0) FM_LOG_WARNING("ERROR writing to socket");
            close(newsockfd);
            return;
        } else {
            n = write(newsockfd, "I got your message", 18);
            if (n < 0) FM_LOG_WARNING("ERROR writing to socket");

            run();
        }
    }

    close(newsockfd);
}

void run() {
    pid_t pid = fork();
    if (pid < 0) {
        FM_LOG_FATAL("fork judger failed: %s", strerror(errno));
    } else if (pid == 0) {
        execl("/usr/local/bin/powerjudge", 
            "/usr/local/bin/powerjudge", 
            "-s", oj_solution.sid, 
            "-p", oj_solution.pid, 
            "-l", oj_solution.language, 
            "-t", oj_solution.time_limit, 
            "-m", oj_solution.memory_limit, 
            "-d", oj_config.temp_dir, 
            "-D", oj_config.data_dir, 
            NULL);
        FM_LOG_FATAL("exec error");
    } else {
        // TODO: wait judger and update result to DataBase
        int status = 0;
        FM_LOG_TRACE("pid=%d", pid);
        if (waitpid(pid, &status, WUNTRACED) == -1) {
          FM_LOG_FATAL("waitpid for judger failed: %s", strerror(errno));
        }

        if (WIFEXITED(status)) {  // normal termination
          if (EXIT_SUCCESS == WEXITSTATUS(status)) {
            FM_LOG_DEBUG("judge succeeded");
            // TODO: check result
          } else if (EXIT_JUDGE == WEXITSTATUS(status)) {
            FM_LOG_TRACE("judge error");
            // SE
          } else {
            FM_LOG_TRACE("judge error");
            // SE
          }
        } else {
          if (WIFSIGNALED(status)) {  // killed by signal
            int signo = WTERMSIG(status);
            FM_LOG_WARNING("judger killed by signal: %s", strsignal(signo));
            // SE
          } else if (WIFSTOPPED(status)) {  // stopped by signal
            int signo = WSTOPSIG(status);
            FM_LOG_FATAL("judger stopped by signal: %s\n", strsignal(signo));
            // SE
          } else {
            FM_LOG_FATAL("judger stopped with unknown reason, status(%d)", status);
            // SE
          }
        }
    }
}

void check_pid() {
    pid_t otherpid;

    FM_LOG_NOTICE("pid file: %s", PID_FILE);
    pfh = pidfile_open(PID_FILE, 0644, &otherpid);
    if (pfh == NULL) {
        if (errno == EEXIST) {
            FM_LOG_FATAL("Daemon already running, pid: %jd", (intmax_t)otherpid);
            exit(EXIT_FAILURE);
        }

        /* If we cannot create pidfile from other reasons, only warn. */
        FM_LOG_WARNING("Cannot open or create pidfile");
    }
}

void read_config(const char *cfg_file) {
    FM_LOG_NOTICE("config file: %s", cfg_file);
    FILE * fp = fopen(cfg_file, "r");
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    if (fp == NULL) {
        error("cannot open configuration file");
    }

    char *key;
    char *value;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (read > 0) {
            int val_len = split(line, &key, &value);
            if (val_len == 0) {
                continue;
            }

            if (strcmp("ip", key) == 0) {
                strncpy(oj_config.ip, value, val_len);
            } else if (strcmp("port", key) == 0) {
                oj_config.port = atoi(value);
            } else if (strcmp("backlog", key) == 0) {
                oj_config.backlog = atoi(value);
            } else if (strcmp("password", key) == 0) {
                strncpy(oj_config.password, value, val_len);
            } else if (strcmp("data.dir", key) == 0) {
                strncpy(oj_config.data_dir, value, val_len);
            } else if (strcmp("temp.dir", key) == 0) {
                strncpy(oj_config.temp_dir, value, val_len);
            }
        }
    }

    if (oj_config.port <= 0) {
        oj_config.port = DEFAULT_PORT;
    }
    if (oj_config.backlog <= 0) {
        oj_config.backlog = DEFAULT_BACKLOG;
    }

    fclose(fp);
    if (line) {
        free(line);
    }
}

int split(char *line, char **key, char **value) {
    int index = 0;
    int val_len = 0;
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

int check_password(char *password, char *message) {
    return strncmp(password, message, strlen(password)) == 0;
}

int parse_arguments(char *str) {
    int number = sscanf(str, "%s %s %s %s %s", oj_solution.sid, oj_solution.pid, oj_solution.language, oj_solution.time_limit, oj_solution.memory_limit);
    FM_LOG_TRACE("sid=%s  pid=%s  language=%s  timeLimit=%s ms  memoryLimit=%s KB", 
        oj_solution.sid, oj_solution.pid, oj_solution.language, oj_solution.time_limit, oj_solution.memory_limit);
    if (number < 5) {
        return -1;
    }
    return 0;
}
