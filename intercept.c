#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/stddef.h>
#include <linux/mm.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
//#include "kill_wrapper.h"



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emil Gulayev & Dennis Vashevnikov");

char *sValue="Bill";

module_param(sValue, charp, S_IRUGO);
MODULE_PARM_DESC(sValue, "A character string");

void** sys_call_table = NULL;

asmlinkage long (*original_syscall)(int pid, int sig);

asmlinkage long our_sys_kill(int pid, int sig){
    struct task_struct *tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
    char *name= tsk->comm;
    printk("welcome to our kill wrapper!");
    if((strcmp(sValue,"Bill")==0 ||strcmp(sValue,name)==0) && sig==9){
        printk("cant kill Bill");
        return -EPERM;
    }

//    return original_syscall(pid,sig);
    return 9;
}
/*
turns on the R/W flag for addr.
*/
void allow_rw(unsigned long addr){
////    write_cr0(read_cr0() & (~addr));
//    unsigned int level;
//    pte_t *pte = lookup_address(addr, &level);
//
//    if(pte->pte &~ _PAGE_RW) pte->pte |= _PAGE_RW;

    unsigned int level;
    pte_t *pte;

    pte = lookup_address(addr, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;

    __flush_tlb();
}

/*
turns off the R/W flag for addr.
*/
void disallow_rw(unsigned long addr) {
//    write_cr0(read_cr0() | addr);

//    unsigned int level2;
//    pte_t *pte = lookup_address(addr, &level2);
//
//    if(pte->pte &~ _PAGE_RW) pte->pte |= _PAGE_RW;
    write_cr0 (read_cr0 () | 0x10000);
}



/*
This function updates the entry of the kill system call in the system call table to point to our_syscall. 
*/
void plug_our_syscall(void){
//    allow_rw((long)sys_call_table[58]);
    allow_rw((unsigned long)sys_call_table[58]);
    original_syscall= (void*)sys_call_table[58];
    sys_call_table[58] = (unsigned long*)&our_sys_kill;
}

/*
This function updates the entry of the kill system call in the system call table to point to the original kill system call. 
*/
void unplug_our_syscall(void){
    sys_call_table[58] = (void*)original_syscall;
//    disallow_rw((long)sys_call_table[58]);
    disallow_rw((unsigned long)sys_call_table[58]);
}



/*
This function is called when loading the module (i.e insmod <module_name>)
*/
int init_module(void) {
    printk("hello to E&D Moudule\n");
    sys_call_table = (void**)kallsyms_lookup_name("sys_call_table");
    plug_our_syscall();
    return 0;
}

void cleanup_module(void) {
//   unplug_our_syscall();
    printk("good bye from module\n");

}
