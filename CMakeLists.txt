cmake_minimum_required(VERSION 3.10)
project(PowerJudge)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/include/mysql)

add_executable(powerjudge
        src/judge.cpp
        src/log.cpp
        src/syscalls.cpp
        src/read_config.cpp
        src/misc.cpp
        src/db_updater.cpp)
target_link_libraries(powerjudge mysqlclient)

add_executable(powerjudged
        src/judged.cpp
        src/thread_safe_queue.hpp
        src/log.cpp
        src/read_config.cpp
        src/misc.cpp)

target_link_libraries(powerjudged curl bsd pthread)


