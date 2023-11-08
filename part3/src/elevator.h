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


#include <linux/delay.h>
#include <linux/random.h>
#include <linux/kthread.h> // Added for kthread_should_stop

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Elevator Kernel Module");

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 1000
#define PERMS 0644
#define PARENT NULL

// Define states for the elevator
#define OFFLINE 0
#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4

static char *message;

static struct proc_ops fops;

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int start_floor, int destination_floor, int type);
extern int (*STUB_stop_elevator)(void);

// Add declarations for missing functions
extern void printFloorList(int floor);
extern int elevatorCount(void);
extern int FloorCountTotal(void);
extern int passengersServiced(void);
int print_passengers(void);
int delete_passengers(int type);
static int elevator_thread_function(void *data);


int issue_request(int start_floor, int destination_floor, int type); 
static int elevator_thread_function(void *data);
int start_elevator(void); 
int stop_elevator(void);

//proc file
int elevator_proc_open(struct inode *sp_inode, struct file *sp_file); 
static ssize_t elevator_proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos); 
int elevator_proc_release(struct inode *sp_inode, struct file *sp_file); 
static int __init elevator_init(void);
static void __exit elevator_exit(void);

// Declare variables for floor_count, num_passengers, and passengers_serviced
static char floor_count[20]; // Initialize as needed

static int elevator_state = OFFLINE;
static int current_floor = 1;
static int elevator_weight = 0;
static int passengers_serviced = 0;
static int read_p;

// int generatr_random(int, int);
// int generate_random_start_floor();
// int generate_random_passenger_type();
// int generate_random_destination_floor();

// Define passenger types
#define FRESHMAN 0
#define SOPHOMORE 1
#define JUNIOR 2
#define SENIOR 3

#define NUM_PASSENGER_TYPES 4
#define MAX_PASSENGERS 5
#define MAX_LOAD 1000

#define ERRORNUM -1

#define PROC_BUF_SIZE 10000

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

Passenger f1, f2, f3, f4, f5, f6;
struct Elevator elevator;

static DECLARE_COMPLETION(elevator_completion);
static DEFINE_MUTEX(elevator_mutex);

// Define mutex for shared data access
static struct mutex elevator_mutex;
struct list_head floor_lists[6];
// Define the elevator thread and its function
static struct task_struct *elevator_thread;

#endif 