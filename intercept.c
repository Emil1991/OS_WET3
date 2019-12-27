#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/stddef.h>
#include <linux/mm.h>
#include <linux/kallsyms.h>
#include "kill_wrapper.h"

// TODO: add more #include statements as necessary

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emil Gulayev & Dennis Vashevnikov");

// TODO: add command-line arguments

void** sys_call_table = NULL;

asmlinkage long (*original_syscall)(int pid, int sig);

asmlinkage long our_sys_kill(int pid, int sig){
    long __res;
    printk("welcome to our kill wrapper!");

    __res=original_syscall(pid,sig);

    if ((__res) < 0) {
//        errno = (-__res);
        return -1;
    }
    return __res;
}
/*
turns on the R/W flag for addr.
*/
void allow_rw(unsigned long addr){
    write_cr0(read_cr0() & (~addr));
}

/*
turns off the R/W flag for addr.
*/
void disallow_rw(unsigned long addr) {
    write_cr0(read_cr0() | addr);
}


/*
This function updates the entry of the kill system call in the system call table to point to our_syscall. 
*/
void plug_our_syscall(void){
    allow_rw((long)sys_call_table[37]);
    original_syscall= (void*)sys_call_table[37];
    sys_call_table[37] = (unsigned long*)our_sys_kill;
}

/*
This function updates the entry of the kill system call in the system call table to point to the original kill system call. 
*/
void unplug_our_syscall(void){
    sys_call_table[37] = (void*)original_syscall;
    disallow_rw((long)sys_call_table[37]);
}



/*
This function is called when loading the module (i.e insmod <module_name>)
*/
int init_module(void) {
    sys_call_table = (void**)kallsyms_lookup_name("sys_call_table");
    plug_our_syscall();
}

void cleanup_module(void) {
   unplug_our_syscall();
}
