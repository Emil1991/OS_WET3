#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/stddef.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/kern_levels.h>
#include <linux/init.h>
#include <linux/mm_types_task.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/paravirt.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emil Gulayev & Dennis Vashevnikov");

char *program_name="Bill";
module_param(program_name, charp, S_IRUGO);
MODULE_PARM_DESC(program_name, "A character string");

struct task_struct;
struct pid_t;

void** sys_call_table = NULL;

asmlinkage int (*original_syscall)(pid_t pid, int sig);

asmlinkage int our_sys_kill(pid_t pid, int sig){
    if((*original_syscall)(pid,0)==1){//process doesnt have permission to kill
        return -EPERM;
    }else if((*original_syscall)(pid,0)!=0){//process doesnt exist
        return -ESRCH;
    }

    struct task_struct* tsk=pid_task(find_vpid(pid), PIDTYPE_PID);
    printk("*tsk:%lu\n",(unsigned long)tsk);
    printk("welcome to our kill wrapper!\n");
    printk("this is the comm:%s\n",tsk->comm);

//    if(tsk->state!=TASK_KILLABLE){
//        return -EPERM;
//    }

//    if(tsk==NULL){
//        return -ESRCH;
//    }

    if(strcmp(tsk->comm,"Bill")==0 || strcmp(program_name,tsk->comm)==0){
        if(sig==9){
            printk("cant kill Bill");
            return -EPERM;
        }
    }

    return (*original_syscall)(pid,sig);
}
/*
turns on the R/W flag for addr.
*/
void allow_rw(unsigned long addr){
    write_cr0(read_cr0() & (~0x10000));
    printk("bitme\n");

//    unsigned int level;
//    pte_t *pte;
//
//    pte = lookup_address(addr, &level);
//    printk("pte adress:%p\n",pte);
//
//    if (pte->pte &~ 0x10000){
//        printk("pte RW");
//        pte->pte |= 0x10000;
//        printk("pte->pte |= 0x10000");
//    }

    __flush_tlb();
}

/*
turns off the R/W flag for addr.
*/
void disallow_rw(unsigned long addr) {
    write_cr0(read_cr0() | 0x10000);
    printk("unbitme\n");

//    unsigned int level2;
//    pte_t *pte = lookup_address(addr, &level2);
//    pte->pte = pte->pte &~ 0x10000;

}



/*
This function updates the entry of the kill system call in the system call table to point to our_syscall. 
*/
void plug_our_syscall(void){
    allow_rw((unsigned long)sys_call_table);
    printk("allowedRW\n");
    original_syscall= (void*)sys_call_table[62];
    sys_call_table[62] = (int*)our_sys_kill;
    printk("plugedIn\n");
    disallow_rw((unsigned long)sys_call_table);
}

/*
This function updates the entry of the kill system call in the system call table to point to the original kill system call. 
*/
void unplug_our_syscall(void){
    allow_rw((unsigned long)sys_call_table);
    sys_call_table[62] = (void*)original_syscall;
    disallow_rw((unsigned long)sys_call_table);
}



/*
This function is called when loading the module (i.e insmod <module_name>)
*/
int init_module(void) {
    printk("hello to E&D Moudule\n");
    sys_call_table = (void**)kallsyms_lookup_name("sys_call_table");
    printk("System call table at %p\n",sys_call_table[62]);
    printk("sending this allow:%lu\n",(unsigned long)sys_call_table);
    printk("swant to send this:%lu\n",(unsigned long)sys_call_table[0]);

    plug_our_syscall();
    return 0;
}

void cleanup_module(void) {
   unplug_our_syscall();
    printk("good bye from module\n");

}


