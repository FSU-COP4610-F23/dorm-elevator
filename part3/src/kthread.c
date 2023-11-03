#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("Example of kernel module proc file for elevator");


// yolo to move the elevator wirth kthreads

#define NUM_FLOORS 6
#define ENTRY_NAME "mover"
#define PERMS 0644
#define PARENT NULL

enum state {OFFLINE, IDLE, LOADING, UP, DOWN};

struct mover {
    int current_floor;
    enum state status;
    struct task_struct *kthread;
};

struct elevator{
    int floors[6];
};

static struct mover mover_thread;
static struct elevator elevator; 

// Use this for up state
int move_to_next_floor(int floor)
{
    return (floor + 1) % NUM_FLOORS;
}

// Use this for down state
int move_to_previous_floor(int floor)
{
    if(floor = 1)    // cant move down from the lobby floor
    {
    return NUM_FLOORS;
    }
    else
    {
    return (floor - 1) % NUM_FLOORS; 
    }
}

void process_mover_state(struct mover * m_thread)
{
    switch(m_thread->status)
    {
    case UP:
        ssleep(2);
        m_thread->current_floor = move_to_next_floor(m_thread->current_floor);
        if(m_thread->current_floor == NUM_FLOORS - 1)
        {
        m_thread->status = LOADING;
        }
        else
        {
        m_thread->status = IDLE; 
        }
        break;
    case DOWN:
        ssleep(2);
        m_thread->current_floor = move_to_previous_floor(m_thread->current_floor);
        if(m_thread->current_floor == 1)
        {
        m_thread->status = LOADING;
        }
        else
        {
        m_thread->status = IDLE;
        }
        break;
    case LOADING:
        ssleep(1);
        m_thread->status = IDLE;
        break;
    default:
        break;
    }    
}

int mover_active(void * _mover)
{
    struct mover * m_thread = (struct mover *) _mover;
    printk(KERN_INFO "mover thread has started running \n");
    while(!mthread_should_stop())            // is this supposed to be here??
    {                        // example has it but with kthread 1x
        process_mover_state(m_thread);
    }
    return 0;

}

int spawn_mover(struct mover * m_thread)
{
    static int current_floor = 0;
    
    m_thread->current_floor = current_floor;
    m_thread->mthread = mthread_run(mover_active, m_thread, "thread mover\n"); // thd spawn here
    
    return 0;
}

int print_elevator_state(char *buf)
{
    int i;
    int len = 0;
    
    const char * states[5] = {"OFFLINE", "IDLE", "LOADING", "UP", "DOWN"};
    
    len += sprintf(buf + len, "Elevator state: %s\n, states[mover_thread.status]);
    for(i = 0; i < NUM_FLOORS; i++
    {
    int floor = i + 1;
    
    len += (i != mover_thread.current_floor)
            ? sprintf(buf + len, "[ ] Floor %d: %d times cleaned. \n", floor, elevator.floors[i])
            : sprintf(buf + len, "[*] Floor %d: %d times cleaned. \n", floor, elevator.floors[i]);    
    }
    return len;
}
// EVERYTHING BELOW IS PROCFILE STUFF



static struct proc_dir_entry* elevator_entry;

static ssize_t elevator_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[10000];
    int len = 0;

    len = sprintf(buf, "Elevator state: \n");
    len += sprintf(buf + len, "Current floor: \n");
    len += sprintf(buf + len, "Current load: \n");
    len += sprintf(buf + len, "Elevator status: \n");
    // you can finish the rest.

    return simple_read_from_buffer(ubuf, count, ppos, buf, len); // better than copy_from_user
}

static const struct proc_ops elevator_fops = {
    .proc_read = elevator_read,
};

static int __init elevator_init(void)
{
    elevator_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &elevator_fops);
    if (!elevator_entry) {
        return -ENOMEM;
    }
    return 0;
}

static void __exit elevator_exit(void)
{
    proc_remove(elevator_entry);
}

module_init(elevator_init);
module_exit(elevator_exit);