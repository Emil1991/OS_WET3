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

static char* program_name="Bill";

module_param(program_name, charp, S_IRUGO);
MODULE_PARM_DESC(program_name, "A character string");

struct task_struct;
struct pid_t;

void** sys_call_table = NULL;

asmlinkage int (*original_syscall)(pid_t pid, int sig);

asmlinkage int our_sys_kill(pid_t pid, int sig){
    struct task_struct* tsk=pid_task(find_vpid(pid), PIDTYPE_PID);
    printk("this is the tsk:%p\n",tsk);

    if((*original_syscall)(pid,0)== -ESRCH || (*original_syscall)(pid,0)==-EPERM){
        return (*original_syscall)(pid,0);
    }else if((strcmp(tsk->comm,"Bill")==0 || strcmp(program_name,tsk->comm)==0) && sig==SIGKILL){
        return -(EPERM);
    }else{
        return (*original_syscall)(pid,sig);
    }
}
/*
turns on the R/W flag for addr.
*/
void allow_rw(unsigned long addr){
    write_cr0(read_cr0() & (~0x10000));
    __flush_tlb();
}

/*
turns off the R/W flag for addr.
*/
void disallow_rw(unsigned long addr) {
    write_cr0(read_cr0() | 0x10000);
}



/*
This function updates the entry of the kill system call in the system call table to point to our_syscall. 
*/
void plug_our_syscall(void){
    allow_rw((unsigned long)sys_call_table);
    original_syscall= (void*)sys_call_table[62];
    sys_call_table[62] = (int*)our_sys_kill;
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
    sys_call_table = (void**)kallsyms_lookup_name("sys_call_table");
    plug_our_syscall();
    return 0;
}

void cleanup_module(void) {
   unplug_our_syscall();
}


