//
// Created by student on 12/20/19.
//
#include <linux/kernel.h>

#ifndef WET3_KILL_WRAPPER_H
#define WET3_KILL_WRAPPER_H

int kill(pid_t pid,int sig) {
    int __res;
    printf("welcome to our kill wrapper!\n");
    __asm__(
    "syscall;"
    : "=a" (__res)
    : "0" (58), "D" (pid),"S" (sig)
    : "memory");

    return __res;
}

#endif //WET3_KILL_WRAPPER_H
