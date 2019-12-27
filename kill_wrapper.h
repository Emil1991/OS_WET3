//
// Created by student on 12/20/19.
//
#include <linux/kernel.h>
#include <stdio.h>



#ifndef WET3_KILL_WRAPPER_H
#define WET3_KILL_WRAPPER_H

int kill(pid_t pid) {
    int __res;
    printf("welcome to our kill wrapper!");

    __asm__(
    "syscall;"
    : "=a" (__res)
    : "0" (37), "D" (pid)
    : "memory");

    if ((__res) < 0) {
//        errno = (-__res);
        return -1;
    }
    return __res;
}


#endif //WET3_KILL_WRAPPER_H
