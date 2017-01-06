#!/usr/bin/env bash

set -e

if [ `id -u` -ne 0 ]; then
    echo "You need root privileges to run this script"
    exit 1
fi

CATALINA_HOME=/usr/share/tomcat8
JUDGE_USER=judge
JUDGE_GROUP=judge
JUDGE_HOME=/home/${JUDGE_USER}

DATA_DIR=${JUDGE_HOME}/data
WORK_DIR=${JUDGE_HOME}/temp

JAVA_POLICY_FILE=${JUDGE_HOME}/java.policy

function create_user() {
    if ! id ${JUDGE_USER} >/dev/null 2>&1 ; then
        echo "try to create user ${JUDGE_USER}."
        useradd --system --home ${JUDGE_HOME} --create-home --user-group \
                --comment "Power Judge" ${JUDGE_USER}
        RET=$?
        if [ ${RET} -ne 0 ]; then
            echo "create user ${JUDGE_USER} failed!"
            return ${RET}
        fi
        mkdir -p ${DATA_DIR}/
        chown ${JUDGE_USER}:${JUDGE_GROUP} ${DATA_DIR}
        mkdir -p ${WORK_DIR}/
        chown ${JUDGE_USER}:${JUDGE_GROUP} ${WORK_DIR}
        echo "create user ${JUDGE_USER} completed."
    else
     echo "user ${JUDGE_USER} already exist."
    fi
}

TOMCAT_USER=
TOMCAT_GROUP=
function find_tomcat() {
  WEBAPPS=${CATALINA_HOME}/webapps
  if [ ! -d "${WEBAPPS}" ]; then
    echo -e "\x1b[31mcannot find tomcat, please set env CATALINA_HOME\x1b[0m" >&2
    return 1
  fi
  TOMCAT_USER=`stat -c '%U' ${WEBAPPS}/`
  TOMCAT_GROUP=`stat -c '%G' ${WEBAPPS}/`
  return 0
}

create_user

if find_tomcat; then
    echo "try to add user ${JUDGE_USER} to tomcat group ${TOMCAT_GROUP}"
    adduser ${JUDGE_USER} ${TOMCAT_GROUP}
    echo "try to add user ${TOMCAT_USER} to judge group ${JUDGE_GROUP}"
    adduser ${TOMCAT_USER} ${JUDGE_GROUP}

    if [ -d /var/log/nginx/ ]; then
        USER=`stat -c '%U' /var/log/nginx/`
        echo "try to add nginx user ${USER} to tomcat group ${TOMCAT_GROUP}"
        usermod -a -G ${TOMCAT_GROUP} ${USER}

        GROUP=`stat -c '%G' /var/log/nginx/`
        echo "try to add tomcat user ${TOMCAT_USER} to nginx group ${GROUP}"
        adduser ${TOMCAT_USER} ${GROUP}
    fi
fi

cp config/judged /etc/init.d/judged && chmod a+x /etc/init.d/judged
update-rc.d judged defaults

[ -d /etc/logrotate.d/ ] && cp config/logrotate /etc/logrotate.d/judged

[ -d /etc/ufw/applications.d/ ] && cp config/ufw /etc/ufw/applications.d/judged

if [ ! -f /etc/judged.conf ]; then
    cp -p config/judged.conf /etc/judged.conf
fi

if [ ! -f ${JAVA_POLICY_FILE} ]; then
  cp config/java.policy ${JAVA_POLICY_FILE}
  chmod 440 ${JAVA_POLICY_FILE}
  chown ${JUDGE_USER}:${JUDGE_GROUP} ${JAVA_POLICY_FILE}
fi

service judged stop
make clean install
service judged start
service judged status
