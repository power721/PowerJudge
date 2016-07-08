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
        fatal_error("ERROR opening socket");
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
        fatal_error("setsockopt SO_REUSEADDR failed");
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fatal_error("ERROR on binding");
    }
    FM_LOG_TRACE("bind");

    clilen = sizeof(cli_addr);

    if (listen(sockfd, oj_config.backlog) < 0) {
        fatal_error("ERROR on listening");
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
    ssize_t n = read(newsockfd, buffer, BUFF_SIZE);
    if (n < 0) FM_LOG_WARNING("ERROR reading from socket");

    FM_LOG_NOTICE("Here is the password: %s", buffer);
    if (check_password(oj_config.password, buffer)) {
        FM_LOG_DEBUG("Authentication Ok.");
        n = write(newsockfd, "Authentication Ok.", 18);
        if (n < 0) FM_LOG_WARNING("ERROR writing to socket");

        bzero(buffer, BUFF_SIZE);
        n = read(newsockfd, buffer, BUFF_SIZE);
        if (n <= 0) {
            FM_LOG_WARNING("ERROR reading from socket");
            close(newsockfd);
            return;
        }
        FM_LOG_NOTICE("Here is the message: %s(%d)", buffer, n);

        if (parse_arguments(buffer) < 0) {
            FM_LOG_WARNING("Missing some parameters.");
            n = write(newsockfd, "Missing some parameters.", 24);
            if (n < 0) FM_LOG_WARNING("ERROR writing to socket");
            close(newsockfd);
            return;
        } else {
            n = write(newsockfd, "I got your request.", 19);
            if (n < 0) FM_LOG_WARNING("ERROR writing to socket");

            close(newsockfd);
            run();
            // mysql_close(con);
            return;
        }
    } else {
        FM_LOG_WARNING("Authentication Failed.");
        n = write(newsockfd, "Authentication Failed.", 22);
        if (n < 0) FM_LOG_WARNING("ERROR writing to socket");
    }

    close(newsockfd);
}

void run() {
    if (oj_solution.cid > 0) {
        snprintf(oj_solution.work_dir, PATH_SIZE, "%s/c%d", oj_config.temp_dir, oj_solution.cid);
    } else {
        strncpy(oj_solution.work_dir, oj_config.temp_dir, strlen(oj_config.temp_dir));
    }
    char stderr_file[PATH_SIZE];
    snprintf(stderr_file, PATH_SIZE, "%s/%s/fatal_error.txt", oj_solution.work_dir, oj_solution.sid);
    FM_LOG_NOTICE("/usr/local/bin/powerjudge -s %s -p %s -l %s -t %s -m %s -w %s -D %s",
                  oj_solution.sid, oj_solution.pid, oj_solution.language,
                  oj_solution.time_limit, oj_solution.memory_limit,
                  oj_solution.work_dir, oj_config.data_dir);
    pid_t pid = fork();
    if (pid < 0) {
        FM_LOG_FATAL("fork judger failed: %s", strerror(errno));
        update_system_error(EXIT_FORK_ERROR);
    } else if (pid == 0) {
        stderr = freopen(stderr_file, "a+", stderr);
        execl("/usr/local/bin/powerjudge",
            "/usr/local/bin/powerjudge",
            "-s", oj_solution.sid,
            "-p", oj_solution.pid,
            "-l", oj_solution.language,
            "-t", oj_solution.time_limit,
            "-m", oj_solution.memory_limit,
            "-w", oj_solution.work_dir,
            "-D", oj_config.data_dir,
            NULL);
        FM_LOG_FATAL("exec fatal_error: %s", strerror(errno));
        update_system_error(EXIT_EXEC_ERROR);
    } else {
        int status = 0;
        FM_LOG_TRACE("process ID=%d", pid);
        if (waitpid(pid, &status, WUNTRACED) == -1) {
          FM_LOG_FATAL("waitpid for judger failed: %s", strerror(errno));
        }

        if (WIFEXITED(status)) {  // normal termination
          if (EXIT_SUCCESS == WEXITSTATUS(status)) {
            FM_LOG_DEBUG("judge succeeded");
            update_result();
          } else if (EXIT_JUDGE == WEXITSTATUS(status)) {
            FM_LOG_TRACE("judge fatal_error");
            update_system_error(OJ_SE);
          } else {
            FM_LOG_TRACE("judge fatal_error");
            update_system_error(WEXITSTATUS(status));
          }
        } else {
          if (WIFSIGNALED(status)) {  // killed by signal
            int signo = WTERMSIG(status);
            FM_LOG_WARNING("judger killed by signal: %s", strsignal(signo));
            update_system_error(signo);
          } else if (WIFSTOPPED(status)) {  // stopped by signal
            int signo = WSTOPSIG(status);
            FM_LOG_FATAL("judger stopped by signal: %s\n", strsignal(signo));
            update_system_error(signo);
          } else {
            FM_LOG_FATAL("judger stopped with unknown reason, status(%d)", status);
            update_system_error(EXIT_UNKNOWN);
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
        fatal_error("cannot open configuration file");
    }

    char *key;
    char *value;
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

    fclose(fp);
    if (line) {
        free(line);
    }
}

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

int check_password(char *password, char *message) {
    if (strlen(password) != strlen(message)) {
        return 0;
    }
    return strncmp(password, message, strlen(password)) == 0;
}

int parse_arguments(char *str) {
    int number = sscanf(str, "%s %d %s %s %s %s", oj_solution.sid, &oj_solution.cid,
                        oj_solution.pid, oj_solution.language,
                        oj_solution.time_limit, oj_solution.memory_limit);
    FM_LOG_TRACE("sid=%s cid=%d  pid=%s  language=%s  timeLimit=%s ms  memoryLimit=%s KB",
                 oj_solution.sid, oj_solution.cid, oj_solution.pid, oj_solution.language,
                 oj_solution.time_limit, oj_solution.memory_limit);
    if (number < 5) {
        return -1;
    }
    return 0;
}

void update_result() {
    char buffer[BUFF_SIZE];
    snprintf(buffer, BUFF_SIZE, "%s/%s/result.txt", oj_solution.work_dir, oj_solution.sid);
    FILE* fp = fopen(buffer, "r");
    if (fp == NULL) {
        FM_LOG_WARNING("cannot open file %s", buffer);
        update_system_error(ERROR_READ_FILE);
        return;
    }

    int number = fscanf(fp, "%d %d %d %d", &oj_solution.result,
                        &oj_solution.time_usage, &oj_solution.memory_usage, &oj_solution.test);
    fclose(fp);
    if (number < 4) {
        FM_LOG_WARNING("read result failed!");
        update_system_error(ERROR_READ_RESULT);
        return;
    }

    char *p = NULL;
    if (oj_solution.result == OJ_CE) {
        snprintf(buffer, BUFF_SIZE, "%s/%s/stderr_compiler.txt", oj_solution.work_dir, oj_solution.sid);
        p = buffer;
    } else if (oj_solution.result == OJ_RE) {
        snprintf(buffer, BUFF_SIZE, "%s/%s/stderr_executive.txt", oj_solution.work_dir, oj_solution.sid);
        p = buffer;
    } else if (oj_solution.result == OJ_SE || oj_solution.result == OJ_RF) {
        snprintf(buffer, BUFF_SIZE, "%s/%s/fatal_error.txt", oj_solution.work_dir, oj_solution.sid);
        p = buffer;
    }

    send_multi_result(p);
}

void update_system_error(int result) {
    FM_LOG_WARNING("system fatal_error %d", result);
    oj_solution.result = OJ_SE;
    char buffer[BUFF_SIZE];
    snprintf(buffer, BUFF_SIZE, "%s/%s/fatal_error.txt", oj_solution.work_dir, oj_solution.sid);
    send_multi_result(buffer);
}

void truncate_upload_file(char* file_path) {
    off_t size = file_size(file_path);
    if (size > MAX_UPLOAD_FILE_SIZE) {
    FM_LOG_TRACE("truncate_upload_file: %s %d %d", file_path, size, MAX_UPLOAD_FILE_SIZE);
        if (truncate(file_path, MAX_UPLOAD_FILE_SIZE) != 0) {
            FM_LOG_WARNING("truncate upload file %s failed: %s", file_path, strerror(errno));
        }
    }
}

void send_multi_result(char* file_path) {
    FM_LOG_TRACE("send_multi_result");
    CURL *curl;

    CURLM *multi_handle;
    int still_running;

    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";
    const size_t size = 25;
    char data[size] = {0};
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "sid",
               CURLFORM_COPYCONTENTS, oj_solution.sid,
               CURLFORM_END);

    snprintf(data, size, "%d", oj_solution.cid);
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "cid",
               CURLFORM_COPYCONTENTS, data,
               CURLFORM_END);

    snprintf(data, size, "%d", oj_solution.result);
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "result",
               CURLFORM_COPYCONTENTS, data,
               CURLFORM_END);

    snprintf(data, size, "%d", oj_solution.time_usage);
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "time",
               CURLFORM_COPYCONTENTS, data,
               CURLFORM_END);

    snprintf(data, size, "%d", oj_solution.memory_usage);
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "memory",
               CURLFORM_COPYCONTENTS, data,
               CURLFORM_END);

    snprintf(data, size, "%d", oj_solution.test);
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "test",
               CURLFORM_COPYCONTENTS, data,
               CURLFORM_END);

  if (file_path != NULL) {
    truncate_upload_file(file_path);
    FM_LOG_NOTICE("will upload fatal_error file %s", file_path);
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "error",
               CURLFORM_FILE, file_path,
               CURLFORM_END);
  }

  FM_LOG_TRACE("try curl_easy_init");
  curl = curl_easy_init();
  multi_handle = curl_multi_init();

  headerlist = curl_slist_append(headerlist, buf);
  if (curl && multi_handle) {
    FM_LOG_TRACE("curl_multi_init OK");
    curl_easy_setopt(curl, CURLOPT_URL, oj_config.api_url);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    curl_multi_add_handle(multi_handle, curl);

    curl_multi_perform(multi_handle, &still_running);

    do {
      struct timeval timeout;
      int rc; /* select() return code */
      CURLMcode mc; /* curl_multi_fdset() return code */

      fd_set fdread;
      fd_set fdwrite;
      fd_set fdexcep;
      int maxfd = -1;

      long curl_timeo = -1;

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);
      FD_ZERO(&fdexcep);

      /* set a suitable timeout to play around with */
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      curl_multi_timeout(multi_handle, &curl_timeo);
      if (curl_timeo >= 0) {
        timeout.tv_sec = curl_timeo / 1000;
        if (timeout.tv_sec > 1)
          timeout.tv_sec = 1;
        else
          timeout.tv_usec = (curl_timeo % 1000) * 1000;
      }

      /* get file descriptors from the transfers */
      mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

      if (mc != CURLM_OK) {
        fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
        break;
      }

      if (maxfd == -1) {
#ifdef _WIN32
        Sleep(100);
        rc = 0;
#else
        /* Portable sleep for platforms other than Windows. */
        struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
        rc = select(0, NULL, NULL, NULL, &wait);
#endif
      } else {
        /* Note that on some platforms 'timeout' may be modified by select().
           If you need access to the original value save a copy beforehand. */
        rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
      }

      switch (rc) {
      case -1:
        /* select fatal_error */
        break;
      case 0:
      default:
        /* timeout or readable/writable sockets */
        curl_multi_perform(multi_handle, &still_running);
        break;
      }
    } while (still_running);

    curl_multi_cleanup(multi_handle);

    curl_easy_cleanup(curl);

    curl_formfree(formpost);

    curl_slist_free_all(headerlist);
    FM_LOG_DEBUG("send_multi_result finished");
  } else {
    FM_LOG_FATAL("cannot init curl: %s", strerror(errno));
    update_system_error(EXIT_CURL_ERROR);
  }
}
