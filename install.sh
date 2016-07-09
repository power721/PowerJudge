#!/bin/bash

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
    if id judge >/dev/null ; then
        echo "create user judge"
        useradd --system --home ${JUDGE_HOME} --create-home --comment "Power Judge" \
                --groups ${JUDGE_GROUP} ${JUDGE_USER}
        RET=$?
        if [ ${RET} -ne 0 ]; then
            echo "create user ${JUDGE_USER} failed!"
            exit ${RET}
        fi
        mkdir -p ${DATA_DIR}/
        mkdir -p ${WORK_DIR}/
    fi
}

TOMCAT_USER=
TOMCAT_GROUP=
function find_tomcat() {
  WEBAPPS=${CATALINA_HOME}/webapps
  if [ ! -d "${WEBAPPS}" ]; then
    echo "cannot find tomcat, please set env CATALINA_HOME"
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

make install
cp java.policy ${WORK_DIR}/
cp install/judged /etc/init.d/judged && chmod a+x /etc/init.d/judged
update-rc.d judged defaults

cp install/logrotate /etc/logrotate.d/judged
cp install/ufw /etc/ufw/applications.d/judged

if [ ! -f ${JUDGE_HOME}/judge.properties ]; then
    cp -p judge.properties ${JUDGE_HOME}/
fi

service judged restart
service judged status
