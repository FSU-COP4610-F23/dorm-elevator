#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/list.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Elevator Kernel Module");

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 1000
#define PERMS 0644
#define PARENT NULL

static struct proc_ops fops;

static char *message;
static struct task_struct *elevator_thread;
static struct mutex elevator_mutex;

enum ElevatorState {
    OFFLINE,
    IDLE,
    LOADING,
    UP,
    DOWN
};

static enum ElevatorState elevator_state = OFFLINE;
static int current_floor = 1;
static int elevator_weight = 0;
static int passengers_serviced = 0;
static int read_p;

#define FRESHMAN 0
#define SOPHOMORE 1
#define JUNIOR 2
#define SENIOR 3

#define NUM_PASSENGER_TYPES 4
#define MAX_PASSENGERS 5

struct Passenger {
    char id[3];
    int weight;
    int destination;
    struct list_head list;
};

struct Floor {
    int total_cnt;
    int total_weight;
    struct list_head list;
};

static struct Floor floors[6];
static struct list_head elevator_passengers;

static int issue_request(int start, int dest, int type) {
    int weight;
    struct Passenger *p;
    char id[3];

    switch (type) {
        case FRESHMAN:
            weight = 100;
            strcpy(id, "F");
            break;
        case SOPHOMORE:
            weight = 150;
            strcpy(id, "O");
            break;
        case JUNIOR:
            weight = 200;
            strcpy(id, "J");
            break;
        case SENIOR:
            weight = 250;
            strcpy(id, "S");
            break;
        default:
            return 1; // Invalid type
    }

    p = kmalloc(sizeof(struct Passenger), GFP_KERNEL);
    if (!p)
        return -ENOMEM;

    strcpy(p->id, id);
    p->weight = weight;
    p->destination = dest;

    if (start >= 1 && start <= 6) {
        struct Floor *floor = &floors[start - 1];
        list_add_tail(&p->list, &floor->list);
        floor->total_cnt++;
        floor->total_weight += weight;
        return 0; // Success
    }

    kfree(p);
    return 1; // Invalid start floor
}

static int elevator_thread_function(void *data) {
    while (!kthread_should_stop()) {
        // Elevator control logic here
        switch (elevator_state) {
            case OFFLINE:
                // Handle offline state logic
                break;
            case IDLE:
                // Handle idle state logic
                break;
            case LOADING:
                // Handle loading state logic
                break;
            case UP:
                // Handle up state logic
                break;
            case DOWN:
                // Handle down state logic
                break;
        }

        msleep(1000); // Sleep for 1 second (adjust as needed)
    }

    return 0;
}

static int passenger_proc_open(struct inode *sp_inode, struct file *sp_file) {
    read_p = 1;
    message = kmalloc(ENTRY_SIZE, GFP_KERNEL);
    if (!message) {
        printk(KERN_WARNING "elevator_proc_open");
        return -ENOMEM;
    }
    return 0;
}

static ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {
    read_p = !read_p;
    if (read_p)
        return 0;

    // Build elevator status and passenger information here

    int len = strlen(message);
    if (copy_to_user(buf, message, len) != 0)
        return -EFAULT;
    
    return len;
}

static int elevator_init(void) {
    fops.proc_open = passenger_proc_open;
    fops.proc_read = elevator_proc_read;

    if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
        printk(KERN_WARNING "elevator_init\n");
        remove_proc_entry(ENTRY_NAME, NULL);
        return -ENOMEM;
    }

    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;

    mutex_init(&elevator_mutex);

    // Initialize floors and elevator_passengers
    for (int i = 0; i < 6; i++) {
        INIT_LIST_HEAD(&floors[i].list);
    }
    INIT_LIST_HEAD(&elevator_passengers);

    // Create and start the elevator thread
    elevator_thread = kthread_run(elevator_thread_function, NULL, "elevator_thread");
    if (IS_ERR(elevator_thread)) {
        printk("Error creating elevator thread\n");
        return PTR_ERR(elevator_thread);
    }

    return 0;
}
module_init(elevator_init);

static void elevator_exit(void) {
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    remove_proc_entry(ENTRY_NAME, NULL);

    // Stop and clean up the elevator thread
    if (elevator_thread)
        kthread_stop(elevator_thread);

    // Cleanup any dynamically allocated memory, like passengers

    // Destroy the mutex and linked lists

}
module_exit(elevator_exit);
