//
// Created by w703710691d on 18-8-24.
//

#include "db_updater.h"
#include <mysql.h>
#include "read_config.h"
#include "log.h"

MYSQL mysql;
bool mysqlOK = false;

bool init_connet() {
    mysql_init(&mysql);
    int value = 1;
    mysqlOK = true;
    read_config(DEFAULT_CFG_FILE);
    mysql_options(&mysql, MYSQL_OPT_RECONNECT, (char *) &value);
    if (!mysql_real_connect(&mysql, oj_config.db_host, oj_config.db_user, oj_config.db_password, oj_config.db_database,
                            oj_config.db_port, nullptr, 0)) {
        FM_LOG_FATAL("connect mysql fail!");
        mysqlOK = false;
    }
    return mysqlOK;
}

void update_solution_status(int cid, int sid, int result, int test) {
    FM_LOG_DEBUG("mysql is %d", (int)mysqlOK);
    if (!mysqlOK) {
        return;
    }
    char buf[BUFF_SIZE];
    if (cid > 0) {
        snprintf(buf, BUFF_SIZE, "update contest_solution set result=%d,test=%d where sid=%d", result, test, sid);
        FM_LOG_DEBUG(buf);
        mysql_query(&mysql, buf);
    } else {
        snprintf(buf, BUFF_SIZE, "update solution set result=%d,test=%d where sid=%d", result, test, sid);
        FM_LOG_DEBUG(buf);
        mysql_query(&mysql, buf);
    }
}

void close_connet() {
    mysql_close(&mysql);
    mysql_library_end();
    mysqlOK = false;
}



