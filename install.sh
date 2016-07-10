#!/usr/bin/env bash

set -e

if [ `id -u` -ne 0 ]; then
    echo "You need root privileges to run this script"
    exit 1
fi

JUDGE_USER=judge
JUDGE_GROUP=judge
JUDGE_HOME=/home/${JUDGE_USER}

DATA_DIR=${JUDGE_HOME}/data
WORK_DIR=${JUDGE_HOME}/temp

JAVA_POLICY_FILE=${WORK_DIR}/java.policy
CONFIG_FILE=${JUDGE_HOME}/judge.properties

function create_user() {
    if ! id ${JUDGE_USER} >/dev/null 2>&1 ; then
        echo "try to create user ${JUDGE_USER}."
        useradd --system --home ${JUDGE_HOME} --create-home --user-group \
                --comment "Power Judge" ${JUDGE_USER}
        RET=$?
        if [ ${RET} -ne 0 ]; then
            echo "create user ${JUDGE_USER} failed!"
            exit ${RET}
        fi
        mkdir -p ${DATA_DIR}/
        mkdir -p ${WORK_DIR}/
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
    echo "cannot find tomcat, please set env CATALINA_HOME" >&2
    return 1
  fi
  TOMCAT_USER=`stat -c '%U' ${WEBAPPS}/`
  TOMCAT_GROUP=`stat -c '%G' ${WEBAPPS}/`
  return 0
}

create_user

if find_tomcat; then
    echo "try to add user ${JUDGE_USER} to group ${TOMCAT_GROUP}"
    adduser ${JUDGE_USER} ${TOMCAT_GROUP}
    echo "try to add user ${TOMCAT_USER} to group ${JUDGE_GROUP}"
    adduser ${TOMCAT_USER} ${JUDGE_GROUP}
fi

cp java.policy ${WORK_DIR}/
cp install/judged /etc/init.d/judged && chmod a+x /etc/init.d/judged
update-rc.d judged defaults

[ -d /etc/logrotate.d/ ] && cp install/logrotate /etc/logrotate.d/judged

[ -d /etc/ufw/applications.d/ ] && cp install/ufw /etc/ufw/applications.d/judged

if [ ! -f /etc/judged.conf ]; then
    cp -p install/judged.conf /etc/judged.conf
fi

service judged stop
make clean install
service judged start
service judged status
