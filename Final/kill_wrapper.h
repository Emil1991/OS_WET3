//
// Created by student on 12/20/19.
//


#ifndef WET3_KILL_WRAPPER_H
#define WET3_KILL_WRAPPER_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <errno.h>
#include <stdio.h>
#include <sched.h>

int kill(pid_t pid,int sig) {
    int __res;
    printf("welcome to our kill wrapper!\n");
    __asm__(
    "syscall;"
    : "=a" (__res)
    : "0" (62), "D" (pid),"S" (sig)
    : "memory");

    if ((__res) < 0) {
        errno = (-__res);
//        return -1;
    }

    return __res;
}

#endif //WET3_KILL_WRAPPER_H
