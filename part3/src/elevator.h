#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h> 
#include <linux/random.h>
#include <linux/list.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/kthread.h> // Added for kthread_should_stop

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Elevator Kernel Module");

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 1000
#define PERMS 0644
#define PARENT NULL

// Elevator status
#define OFFLINE 0
#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4

// passenger types
#define FRESHMAN 0
#define SOPHOMORE 1
#define JUNIOR 2
#define SENIOR 3

#define NUM_PASSENGER_TYPES 4
#define MAX_PASSENGERS 5
#define MAX_LOAD 750

#define ERRORNUM -1
#define PROC_BUF_SIZE 10000

extern struct Elevator elevator;
extern struct list_head floor_lists[6];
extern struct task_struct *elevator_thread;
extern char floor_count[6];
extern int elevator_state;
extern int elevator_dest;
extern int current_floor;
extern int elevator_weight;
extern int elevator_count;
extern int passengers_serviced;
extern char *message;

struct Elevator
{
    int total_cnt;
    int total_weight;
    struct list_head list;
};

typedef struct passenger
{
    char id[3];
    int weight;
    int destination;
    char type;
    struct list_head list;
} Passenger;

int issue_request(int start_floor, int destination_floor, int type); 
void load_passengers(int current_floor);
void unload_passengers(void);
void searchNextEmpty(void);
int start_elevator(void); 
int stop_elevator(void);
int exit_elevator(void);
extern int FloorCountTotal(void);

// static struct proc_ops fops;

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int start_floor, int destination_floor, int type);
extern int (*STUB_stop_elevator)(void);


int __init elevator_init(void);
void __exit elevator_exit(void);
int elevator_thread_function(void *data);


static DECLARE_COMPLETION(elevator_completion);
static DEFINE_MUTEX(elevator_mutex);

#endif 