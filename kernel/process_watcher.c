#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/kthread.h> // for threads
#include <linux/time.h>    // for using jiffies
#include <linux/timer.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel Belz <daniel.belz@pace.com>");
MODULE_DESCRIPTION("process usage watcher for linux devices");

#define DELAY 15

#ifdef CONFIG_KALLSYMS
/*
 * Provides a wchan file via kallsyms in a proper one-value-per-file format.
 * Returns the resolved symbol.  If that fails, simply return the address.
 */
static int proc_pid_wchan(struct task_struct *task, char *buffer)
{
    unsigned long wchan;
    char symname[KSYM_NAME_LEN];

    wchan = get_wchan(task);

    if (lookup_symbol_name(wchan, symname) < 0)
        return sprintf(buffer, "%lu", wchan);
    else
        return sprintf(buffer, "%s", symname);
}
#endif /* CONFIG_KALLSYMS */

static struct task_struct *pwth;
int processwatcher_th(void)
{
    struct task_struct *task;
    /* cpu utilisation of the process */
    unsigned long cpu = 0; //, cputot=0;
    /* process pid */
    pid_t pid;
    int delay = HZ * DELAY;
    unsigned long *lusg;
    unsigned long *lwch;
    unsigned long jnow, jprev;
    unsigned long wchan = ~0UL;
    struct task_struct *TSK;
    struct sched_param PARAM = {.sched_priority = MAX_RT_PRIO};
    TSK = current;

    PARAM.sched_priority = 84;
    sched_setscheduler(TSK, SCHED_RR, &PARAM);

    lusg = kmalloc(sizeof(unsigned long) * 32768, GFP_KERNEL);
    lwch = kmalloc(sizeof(unsigned long) * 32768, GFP_KERNEL);
    printk(KERN_INFO "Running processwatcher thread running every %d seconds\n", DELAY);

    jnow = jiffies;
    jprev = 0;

    while (1) {
        jprev = jnow;
        msleep(delay);
        jnow = jiffies;
        read_lock(&tasklist_lock);
        for_each_process(task)
        {
            /*put the other info  of the process*/
            pid = task->pid;
            /* cpu utilisation of the process */
            cpu = (((task->utime + task->stime) - lusg[pid]) * 100) / (jnow - jprev);
            //cputot+=cpu;
            wchan = get_wchan(task);
            if (wchan == lwch[pid]) {
                char wchanstr[128];
                proc_pid_wchan(task, &wchanstr);
                printk(KERN_WARNING "[PW WARNING] WAITING TASK %s pid %d usage %lu%%, STILL WAITING AT %s th[now=%lu, snapshot=%lu last run=%lu]\n",
                       task->comm, pid, cpu, wchanstr, jiffies, jnow, jprev);
            }

            if (cpu > 65 && cpu < 200)
                printk(KERN_WARNING "[PW WARNING] USAGE TASK %s pid %d usage %lu%%, th[now=%lu, snapshot=%lu last run=%lu]\n", task->comm, pid, cpu, jiffies, jnow, jprev);

            lwch[pid] = wchan;
            lusg[pid] = task->utime + task->stime;
        }
        read_unlock(&tasklist_lock);
        //if (cputot > 95 && cputot < 200)
        //	printk(KERN_WARNING "WARNING [PW] CPU TOTAL USAGE IS HIGH %lu%%\n", cputot);
        //cputot = 0;
    }
    kfree(lusg);
    return 0;
}

static int __init processwatcher_init_module(void)
{
    char pw_thread[8] = "pwatch";
    printk(KERN_INFO "processwatcher: loading.\n");
    pwth = kthread_create(processwatcher_th, NULL, pw_thread);
    if ((pwth)) {
        wake_up_process(pwth);
    }
    return 0;
}

static void __exit processwatcher_cleanup_module(void)
{
    printk(KERN_INFO "processwatcher: unload.\n");
}

module_init(processwatcher_init_module);
module_exit(processwatcher_cleanup_module);