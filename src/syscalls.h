/*
 * Copyright 2014 power <power0721#gmail.com>
 * PowerOJ GPLv2
 */
#ifndef SRC_SYSCALLS_H_
#define SRC_SYSCALLS_H_

#include "log.h"
#include "judge_core.h"

/*
 * syscalls 每个值对应的是该syscall可被调用的次数
 *    取值有3种:
 *      正值: 表示可被调用的次数, 每次调用后会减一(比如fork)
 *      零值: 表示禁止调用(比如open)
 *      负值: 表示不限制该syscall(比如write)
 *
 * syscalls的初始化由init_syscalls函数完成
 */
static int syscalls[1024] = {0};

/*
 * SC_* 数组对是用于初始化syscalls的数据来源,
 * 每两个数字为一组k/v: syscall_id:次数
 *   次数 < 0 表示不限制
 *   次数 > 0 表示可调用次数, 运行时每调用一次减一
 *   次数 = 0 (运行时达到) 不再允许调用该syscall
 * 最后一个syscall_id存放的是SYSCALLS_END，表示结束
 * 未指定的syscall_id的次数将被自动初始化为0
 */
#define SYSCALLS_INFINITE -1
#define SYSCALLS_END      -2
#ifdef __i386
// http://docs.cs.up.ac.za/programming/asm/derick_tut/syscalls.html
// http://syscalls.kernelgrok.com/

// C or C++
int SC_C[512] =
{
  252,                  2,
  SYS_access,           SYSCALLS_INFINITE,
  SYS_brk,              SYSCALLS_INFINITE,
  SYS_close,            SYSCALLS_INFINITE,
  SYS_execve,           1,
  SYS_fstat64,          SYSCALLS_INFINITE,
  SYS_mmap2,            SYSCALLS_INFINITE,
  SYS_mprotect,         SYSCALLS_INFINITE,
  SYS_munmap,           SYSCALLS_INFINITE,
  SYS_read,             SYSCALLS_INFINITE,
  SYS_readlink,         SYSCALLS_INFINITE,
  SYS_set_thread_area,  SYSCALLS_INFINITE,
  SYS_time,             SYSCALLS_INFINITE,
  SYS_uname,            SYSCALLS_INFINITE,
  SYS_write,            SYSCALLS_INFINITE,
  SYSCALLS_END
};

// Pascal
int SC_PASCAL[512] =
{
  13,                   SYSCALLS_INFINITE,
  175,                  SYSCALLS_INFINITE,
  191,                  SYSCALLS_INFINITE,
  195,                  SYSCALLS_INFINITE,
  252,                  2,
  91,                   SYSCALLS_INFINITE,
  SYS_brk,              SYSCALLS_INFINITE,
  SYS_execve,           1,
  SYS_getrlimit,        SYSCALLS_INFINITE,
  SYS_ioctl,            SYSCALLS_INFINITE,
  SYS_mmap,             SYSCALLS_INFINITE,
  SYS_open,             SYSCALLS_INFINITE,
  SYS_read,             SYSCALLS_INFINITE,
  SYS_readlink,         SYSCALLS_INFINITE,
  SYS_rt_sigaction,     SYSCALLS_INFINITE,
  SYS_set_thread_area,  SYSCALLS_INFINITE,
  SYS_uname,            SYSCALLS_INFINITE,
  SYS_write,            SYSCALLS_INFINITE,
  SYSCALLS_END
};

// Java
int SC_JAVA[512] =
{
  295,                  SYSCALLS_INFINITE,
  SYS_access,           SYSCALLS_INFINITE,
  SYS_brk,              SYSCALLS_INFINITE,
  SYS_clone,            SYSCALLS_INFINITE,
  SYS_close,            SYSCALLS_INFINITE,
  SYS_close,            SYSCALLS_INFINITE,
  SYS_execve,           SYSCALLS_INFINITE,
  SYS_exit_group,       SYSCALLS_INFINITE,
  SYS_fcntl64,          SYSCALLS_INFINITE,
  SYS_fstat64,          SYSCALLS_INFINITE,
  SYS_futex,            SYSCALLS_INFINITE,
  SYS_getdents64,       SYSCALLS_INFINITE,
  SYS_getegid32,        SYSCALLS_INFINITE,
  SYS_geteuid32,        SYSCALLS_INFINITE,
  SYS_getgid32,         SYSCALLS_INFINITE,
  SYS_getrlimit,        SYSCALLS_INFINITE,
  SYS_getuid32,         SYSCALLS_INFINITE,
  SYS_mmap2,            SYSCALLS_INFINITE,
  SYS_mprotect,         SYSCALLS_INFINITE,
  SYS_munmap,           SYSCALLS_INFINITE,
  SYS_open,             SYSCALLS_INFINITE,
  SYS_read,             SYSCALLS_INFINITE,
  SYS_readlink,         SYSCALLS_INFINITE,
  SYS_rt_sigaction,     SYSCALLS_INFINITE,
  SYS_rt_sigprocmask,   SYSCALLS_INFINITE,
  SYS_set_robust_list,  SYSCALLS_INFINITE,
  SYS_set_thread_area,  SYSCALLS_INFINITE,
  SYS_set_tid_address,  SYSCALLS_INFINITE,
  SYS_sigprocmask,      SYSCALLS_INFINITE,
  SYS_stat64,           SYSCALLS_INFINITE,
  SYS_ugetrlimit,       SYSCALLS_INFINITE,
  SYS_uname,            SYSCALLS_INFINITE,
  SYS_uname,            SYSCALLS_INFINITE,
  SYSCALLS_END
};

// Python
int SC_PYTHON[512] = {
  102,                  SYSCALLS_INFINITE,
  117,                  SYSCALLS_INFINITE,
  146,                  SYSCALLS_INFINITE,
  158,                  SYSCALLS_INFINITE,
  191,                  SYSCALLS_INFINITE,
  39,                   SYSCALLS_INFINITE,
  60,                   SYSCALLS_INFINITE,
  SYS__llseek,          SYSCALLS_INFINITE,
  SYS_access,           SYSCALLS_INFINITE,
  SYS_brk,              SYSCALLS_INFINITE,
  SYS_close,            SYSCALLS_INFINITE,
  SYS_execve,           SYSCALLS_INFINITE,
  SYS_exit_group,       SYSCALLS_INFINITE,
  SYS_fcntl64,          SYSCALLS_INFINITE,
  SYS_fstat64,          SYSCALLS_INFINITE,
  SYS_futex,            SYSCALLS_INFINITE,
  SYS_getcwd,           SYSCALLS_INFINITE,
  SYS_getdents64,       SYSCALLS_INFINITE,
  SYS_getegid32,        SYSCALLS_INFINITE,
  SYS_geteuid32,        SYSCALLS_INFINITE,
  SYS_getgid32,         SYSCALLS_INFINITE,
  SYS_getrlimit,        SYSCALLS_INFINITE,
  SYS_getuid32,         SYSCALLS_INFINITE,
  SYS_ioctl,            SYSCALLS_INFINITE,
  SYS_lstat64,          SYSCALLS_INFINITE,
  SYS_mmap2,            SYSCALLS_INFINITE,
  SYS_mprotect,         SYSCALLS_INFINITE,
  SYS_mremap,           SYSCALLS_INFINITE,
  SYS_munmap,           SYSCALLS_INFINITE,
  SYS_open,             SYSCALLS_INFINITE,
  SYS_read,             SYSCALLS_INFINITE,
  SYS_readlink,         SYSCALLS_INFINITE,
  SYS_rt_sigaction,     SYSCALLS_INFINITE,
  SYS_rt_sigprocmask,   SYSCALLS_INFINITE,
  SYS_set_robust_list,  SYSCALLS_INFINITE,
  SYS_set_thread_area,  SYSCALLS_INFINITE,
  SYS_set_tid_address,  SYSCALLS_INFINITE,
  SYS_stat64,           SYSCALLS_INFINITE,
  SYS_uname,            SYSCALLS_INFINITE,
  SYS_write,            SYSCALLS_INFINITE,
  SYSCALLS_END
};
#else
// http://blog.rchapman.org/post/36801038863/linux-system-call-table-for-x86-64

// C or C++
int SC_C[512] =
{
  252,                 2,
  SYS_access,          SYSCALLS_INFINITE,
  SYS_arch_prctl,      SYSCALLS_INFINITE,
  SYS_brk,             SYSCALLS_INFINITE,
  SYS_close,           SYSCALLS_INFINITE,
  SYS_execve,          SYSCALLS_INFINITE,
  SYS_exit_group,      SYSCALLS_INFINITE,
  SYS_fstat,           SYSCALLS_INFINITE,
  SYS_mmap,            SYSCALLS_INFINITE,
  SYS_mprotect,        SYSCALLS_INFINITE,
  SYS_munmap,          SYSCALLS_INFINITE,
  SYS_read,            SYSCALLS_INFINITE,
  SYS_readlink,        SYSCALLS_INFINITE,
  SYS_set_thread_area, SYSCALLS_INFINITE,
  SYS_time,            SYSCALLS_INFINITE,
  SYS_uname,           SYSCALLS_INFINITE,
  SYS_write,           SYSCALLS_INFINITE,
  SYSCALLS_END
};

// Pascal
int SC_PASCAL[512] =
{
  158,                 SYSCALLS_INFINITE,
  191,                 SYSCALLS_INFINITE,
  231,                 SYSCALLS_INFINITE,
  252,                 SYSCALLS_INFINITE,
  4,                   SYSCALLS_INFINITE,
  SYS_brk,             SYSCALLS_INFINITE,
  SYS_close,           SYSCALLS_INFINITE,
  SYS_execve,          SYSCALLS_INFINITE,
  SYS_exit_group,      SYSCALLS_INFINITE,
  SYS_getrlimit,       SYSCALLS_INFINITE,
  SYS_ioctl,           SYSCALLS_INFINITE,
  SYS_mmap,            SYSCALLS_INFINITE,
  SYS_munmap,          SYSCALLS_INFINITE,
  SYS_open,            SYSCALLS_INFINITE,
  SYS_read,            SYSCALLS_INFINITE,
  SYS_readlink,        SYSCALLS_INFINITE,
  SYS_rt_sigaction,    SYSCALLS_INFINITE,
  SYS_set_thread_area, SYSCALLS_INFINITE,
  SYS_time,            SYSCALLS_INFINITE,
  SYS_uname,           SYSCALLS_INFINITE,
  SYS_write,           SYSCALLS_INFINITE,
  SYSCALLS_END
};

// Java
int SC_JAVA[512] =
{
  110,                 SYSCALLS_INFINITE,
  111,                 SYSCALLS_INFINITE,
  13,                  SYSCALLS_INFINITE,
  158,                 SYSCALLS_INFINITE,
  16,                  SYSCALLS_INFINITE,
  22,                  SYSCALLS_INFINITE,
  257,                 SYSCALLS_INFINITE,
  33,                  SYSCALLS_INFINITE,
  39,                  SYSCALLS_INFINITE,
  6,                   SYSCALLS_INFINITE,
  61,                  SYSCALLS_INFINITE,
  79,                  SYSCALLS_INFINITE,
  8,                   SYSCALLS_INFINITE,
  SYS_access,          SYSCALLS_INFINITE,
  SYS_brk,             SYSCALLS_INFINITE,
  SYS_clone,           SYSCALLS_INFINITE,
  SYS_close,           SYSCALLS_INFINITE,
  SYS_close,           SYSCALLS_INFINITE,
  SYS_execve,          SYSCALLS_INFINITE,
  SYS_exit_group,      SYSCALLS_INFINITE,
  SYS_fcntl,           SYSCALLS_INFINITE,
  SYS_fstat,           SYSCALLS_INFINITE,
  SYS_futex,           SYSCALLS_INFINITE,
  SYS_getdents64,      SYSCALLS_INFINITE,
  SYS_getegid,         SYSCALLS_INFINITE,
  SYS_geteuid,         SYSCALLS_INFINITE,
  SYS_getgid,          SYSCALLS_INFINITE,
  SYS_getrlimit,       SYSCALLS_INFINITE,
  SYS_getrlimit,       SYSCALLS_INFINITE,
  SYS_getuid,          SYSCALLS_INFINITE,
  SYS_mmap,            SYSCALLS_INFINITE,
  SYS_mprotect,        SYSCALLS_INFINITE,
  SYS_munmap,          SYSCALLS_INFINITE,
  SYS_open,            SYSCALLS_INFINITE,
  SYS_read,            SYSCALLS_INFINITE,
  SYS_readlink,        SYSCALLS_INFINITE,
  SYS_rt_sigaction,    SYSCALLS_INFINITE,
  SYS_rt_sigprocmask,  SYSCALLS_INFINITE,
  SYS_set_robust_list, SYSCALLS_INFINITE,
  SYS_set_thread_area, SYSCALLS_INFINITE,
  SYS_set_tid_address, SYSCALLS_INFINITE,
  SYS_stat,            SYSCALLS_INFINITE,
  SYS_uname,           SYSCALLS_INFINITE,
  SYS_uname,           SYSCALLS_INFINITE,
  SYSCALLS_END
};

// Python
int SC_PYTHON[512] = {
  102,                 SYSCALLS_INFINITE,
  117,                 SYSCALLS_INFINITE,
  146,                 SYSCALLS_INFINITE,
  158,                 SYSCALLS_INFINITE,
  191,                 SYSCALLS_INFINITE,
  257,                 SYSCALLS_INFINITE,
  39,                  SYSCALLS_INFINITE,
  41,                  SYSCALLS_INFINITE,
  42,                  SYSCALLS_INFINITE,
  60,                  SYSCALLS_INFINITE,
  SYS_access,          SYSCALLS_INFINITE,
  SYS_brk,             SYSCALLS_INFINITE,
  SYS_close,           SYSCALLS_INFINITE,
  SYS_execve,          SYSCALLS_INFINITE,
  SYS_exit_group,      SYSCALLS_INFINITE,
  SYS_fcntl,           SYSCALLS_INFINITE,
  SYS_fstat,           SYSCALLS_INFINITE,
  SYS_futex,           SYSCALLS_INFINITE,
  SYS_getcwd,          SYSCALLS_INFINITE,
  SYS_getdents,        SYSCALLS_INFINITE,
  SYS_getegid,         SYSCALLS_INFINITE,
  SYS_geteuid,         SYSCALLS_INFINITE,
  SYS_getgid,          SYSCALLS_INFINITE,
  SYS_getrlimit,       SYSCALLS_INFINITE,
  SYS_getuid,          SYSCALLS_INFINITE,
  SYS_ioctl,           SYSCALLS_INFINITE,
  SYS_lseek,           SYSCALLS_INFINITE,
  SYS_lstat,           SYSCALLS_INFINITE,
  SYS_mmap,            SYSCALLS_INFINITE,
  SYS_mprotect,        SYSCALLS_INFINITE,
  SYS_mremap,          SYSCALLS_INFINITE,
  SYS_munmap,          SYSCALLS_INFINITE,
  SYS_open,            SYSCALLS_INFINITE,
  SYS_read,            SYSCALLS_INFINITE,
  SYS_readlink,        SYSCALLS_INFINITE,
  SYS_rt_sigaction,    SYSCALLS_INFINITE,
  SYS_rt_sigprocmask,  SYSCALLS_INFINITE,
  SYS_set_robust_list, SYSCALLS_INFINITE,
  SYS_set_tid_address, SYSCALLS_INFINITE,
  SYS_stat,            SYSCALLS_INFINITE,
  SYS_write,           SYSCALLS_INFINITE,
  SYSCALLS_END
};
#endif

// 根据 SC_* 数组来初始化syscalls
int init_syscalls(int lang)
{
  int i;
  int *p = NULL;
  switch (lang) {
    case LANG_C:
    case LANG_CPP:
      p = SC_C;
      break;
    case LANG_PASCAL:
      p = SC_PASCAL;
      break;
    case LANG_JAVA:
      p = SC_JAVA;
      break;
    case LANG_PYTHON:
      p = SC_PYTHON;
      break;
    default:
      FM_LOG_FATAL("unknown language id: %d", lang);
      return 1;
  }
  memset(syscalls, 0, sizeof(syscalls));
  for (i = 0; p[i] != SYSCALLS_END; i += 2) {
    syscalls[p[i]] = p[i+1];
  }
  return 0;
}

static bool in_syscall = true;
bool is_valid_syscall(int syscall_id)
{
  in_syscall = !in_syscall;
  if (syscalls[syscall_id] == 0) {
    return false;
  } else if (syscalls[syscall_id] > 0) {
    if (in_syscall == false) {
      syscalls[syscall_id]--;
    }
  } else {
  }
  return true;
}

#endif  // SRC_SYSCALLS_H_
