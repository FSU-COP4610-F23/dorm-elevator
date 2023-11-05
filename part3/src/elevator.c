#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/kthread.h> // Added for kthread_should_stop

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Adds random passengers to a list and uses proc to query the stats");

#define ENTRY_NAME "passenger_list"
#define ENTRY_SIZE 1000
#define PERMS 0644
#define PARENT NULL

static char *message;

static struct proc_ops fops;

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int start_floor, int destination_floor, int type);
extern int (*STUB_stop_elevator)(void);

// Add declarations for missing functions
extern void printFloorList(int floor); // Declare printFloorList function
extern int elevatorCount(void);        // Declare elevatorCount function
extern int FloorCountTotal(void);      // Declare FloorCountTotal function
extern int passengersServiced(void);   // Declare passengersServiced function

// Declare variables for floor_count, num_passengers, and passengers_serviced
static char floor_count[20];         // Initialize as needed
// static char num_passengers[20];      // Initialize as needed
// static char passengers_serviced[20]; // Initialize as needed

#define OFFLINE 0

static int elevator_state = OFFLINE;
static int current_floor = 1;
// static int num_passengers = 0;
static int elevator_weight = 0;
static int passengers_serviced = 0;
static int read_p;

#define FRESHMAN 0
#define SOPHOMORE 1
#define JUNIOR 2
#define SENIOR 3

#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4

#define NUM_PASSENGER_TYPES 4
#define MAX_PASSENGERS 5

struct Elevator
{ // linked list of passengers on elevator & total weight and count
    int total_cnt;
    int total_weight;
    struct list_head list;
};

typedef struct passenger
{ // information of each passenger
    char id[3];
    int weight;
    int destination;
    struct list_head list;
} Passenger; // Passenger can be used as nickname

Passenger f1, f2, f3, f4, f5, f6; // one for each floor
struct Elevator elevator;

static DECLARE_COMPLETION(elevator_completion);
// static struct task_struct *elevator_thread;

// static int elevator_thread_function(void *data)
// {
//     struct Elevator *e_thread = (struct Elevator *)data;

//     while (!kthread_should_stop())
//     {
//         // Elevator control logic here
//         switch (elevator_state)
//         {
//         case OFFLINE:
//             // Handle offline state logic
//             break;
//         case IDLE:
//             // Handle idle state logic
//             break;
//         case LOADING:
//             msleep(1000); // sleep for 1 second
//             e_thread->elevator_state = IDLE;
//             break;
//         case UP:
//             esleep(2000); // sleep for 2 seconds
//             e_thread->current_floor = move_to_next_floor(e_thread->current_floor);
//             if (e_thread->current_floor == NUM_FLOORS - 1)
//             {
//                 e_thread->elevator_state = LOADING
//             }
//             else
//             {
//                 e_thread->elevator_state = IDLE;
//             }
//             break;
//         case DOWN:
//             esleep(2000); // sleep for 2 seconds
//             e_thread->current_floor = move_to_previous_floor(e_thread->current_floor);
//             if (e_thread->current_floor == 1)
//             {
//                 e_thread->elevator_state = LOADING;
//             }
//             else
//             {
//                 e_thread->elevator_state = IDLE;
//             }
//             break;
//         }

//         set_current_state(TASK_INTERRUPTIBLE);
//         schedule_timeout(HZ); // Sleep for 1 second (adjust as needed)
//         esleep(1000); // Sleep for 1 second (adjust as needed)
//     }
//     complete(&elevator_completion);
// }

int issue_request(int start, int dest, int type)
{
    int weight;
    char id[3] = ""; // Increase the size to accommodate "F", "O", "J", or "S"
    Passenger *p = NULL;

    switch (type)
    {
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
        return 1;
    }

    p = kmalloc(sizeof(Passenger), GFP_KERNEL);

    if (p == NULL)
        return -ENOMEM;

    strcpy(p->id, id);
    sprintf(p->id + 1, "%d", dest);

    p->weight = weight;
    p->destination = dest;

    switch (start)
    {
    case 1:
        list_add_tail(&p->list, &f1.list);
        break;
    case 2:
        list_add_tail(&p->list, &f2.list);
        break;
    case 3:
        list_add_tail(&p->list, &f3.list);
        break;
    case 4:
        list_add_tail(&p->list, &f4.list);
        break;
    case 5:
        list_add_tail(&p->list, &f5.list);
        break;
    case 6:
        list_add_tail(&p->list, &f6.list);
        break;
    default:
        return 1;
    }

    return 0;
}

int print_passengers(void)
{
    int i = 0;
    struct list_head *temp;
    Passenger *p;
    char *buf = kmalloc(100, GFP_KERNEL);

    if (buf == NULL)
    {
        printk(KERN_WARNING "print_passengers: Failed to allocate memory\n");
        return -ENOMEM;
    }

    if (message == NULL)
    {
        printk(KERN_WARNING "print_passengers: Message buffer is null\n");
        kfree(buf);
        return -ENOMEM;
    }

    strcpy(message, "");

    sprintf(buf, "Total count is: %d\n", elevator.total_cnt);
    strcat(message, buf);

    sprintf(buf, "Total weight is: %d\n", elevator.total_weight);
    strcat(message, buf);

    sprintf(buf, "Passengers seen:\n");
    strcat(message, buf);

    i = 0;

    list_for_each(temp, &elevator.list)
    {
        p = list_entry(temp, Passenger, list);

        if (i % 5 == 0 && i > 0)
        {
            strcat(message, "\n");
        }

        sprintf(buf, "%s ", p->id);
        strcat(message, buf);

        i++;
    }

    strcat(message, "\n");

    kfree(buf);
    return 0;
}

int delete_passengers(int type)
{
    struct list_head move_list;
    struct list_head *temp;
    struct list_head *dummy;
    int i;
    Passenger *p;

    INIT_LIST_HEAD(&move_list);

    /* Move items to a temporary list to illustrate movement */
    list_for_each_safe(temp, dummy, &elevator.list)
    {
        p = list_entry(temp, Passenger, list);

        if (strcmp(p->id, "F") == 0 && type == FRESHMAN)
        {
            list_move_tail(temp, &move_list); /* Move to the back of the list */
        }
        else if (strcmp(p->id, "O") == 0 && type == SOPHOMORE)
        {
            list_move_tail(temp, &move_list); /* Move to the back of the list */
        }
        else if (strcmp(p->id, "J") == 0 && type == JUNIOR)
        {
            list_move_tail(temp, &move_list); /* Move to the back of the list */
        }
        else if (strcmp(p->id, "S") == 0 && type == SENIOR)
        {
            list_move_tail(temp, &move_list); /* Move to the back of the list */
        }
    }

    /* Print stats of list to syslog, entry version just as an example (not needed here) */
    i = 0;
    list_for_each_entry(p, &move_list, list)
    {
        i++;
    }
    printk(KERN_NOTICE "Passenger type %d had %d entries\n", type, i);

    /* Free up memory allocation of Passengers */
    list_for_each_safe(temp, dummy, &move_list)
    {
        p = list_entry(temp, Passenger, list);
        list_del(temp); /* Removes entry from the list */
        kfree(p);
    }
    return 0;
}

// /********************************************************************/

int passenger_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    read_p = 1;
    message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS); // allocates 1000 bytes to message
    if (message == NULL)
    {
        printk(KERN_WARNING "elevator_proc_open");
        return -ENOMEM;
    }

    return 0;
}

ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{

    read_p = !read_p; // idk what this does
    if (read_p)
        return 0;

    //*****************************idk how kthreads for movement work but maybe they are called here every read? *******************************//
    // call function that updates state
    // call function that moves elevator to next dest
    // drop off corresponding passengers to that floor
    int addToElevator(int current_floor); // while on that floor, add any customers to elevator

    /*****once elevator moves and loads for 1 cycle print all this stuff out*****/

    strcpy(message, "Elevator state: "); // start message to print to procfile

    switch (elevator_state)
    {
    case OFFLINE:
        strcat(message, "OFFLINE"); // adds to message
        break;
    case IDLE:
        strcat(message, "IDLE");
        break;
    case LOADING:
        strcat(message, "LOADING");
        break;
    case UP:
        strcat(message, "UP");
        break;
    case DOWN:
        strcat(message, "DOWN");
        break;
    }
    strcat(message, "\n");

    char current_floor_str[10];
    char elevator_weight_str[10];

    snprintf(current_floor_str, sizeof(current_floor_str), "%d", current_floor);
    snprintf(elevator_weight_str, sizeof(elevator_weight_str), "%d", elevator_weight);

    strcat(message, "Current floor: ");
    strcat(message, current_floor);
    strcat(message, "\n");

    strcat(message, "Current load: ");
    strcat(message, elevator_weight_str);
    strcat(message, " lbs");
    strcat(message, "\n");

    strcat(message, "Elevator status: ");
    printElevator();

    if (current_floor == 6)
        strcat(message, "[*] Floor 6: ");
    else
        strcat(message, "[ ] Floor 6: ");
    printFloorList(6);

    if (current_floor == 5)
        strcat(message, "[*] Floor 5: ");
    else
        strcat(message, "[ ] Floor 5: ");
    printFloorList(5);

    if (current_floor == 4)
        strcat(message, "[*] Floor 4: ");
    else
        strcat(message, "[ ] Floor 4: ");
    printFloorList(4);

    if (current_floor == 3)
        strcat(message, "[*] Floor 3: ");
    else
        strcat(message, "[ ] Floor 3: ");
    printFloorList(3);

    if (current_floor == 2)
        strcat(message, "[*] Floor 2: ");
    else
        strcat(message, "[ ] Floor 2: ");
    printFloorList(2);

    if (current_floor == 1)
        strcat(message, "[*] Floor 1: ");
    else
        strcat(message, "[ ] Floor 1: ");
    printFloorList(1);

    strcat(message, "Number of passengers: ");
    // sprintf(num_passengers, "%d", elevatorCount(); //prints %d into num_passengers
    strcat(message, elevator.total_cnt); // might be able to just do this and ignore prev line

    strcat(message, "Number of passengers waiting: ");
    // sprintf(floor_count, "%d", FloorCountTotal();
    strcat(message, floor_count);

    strcat(message, "Number of passengers serviced: ");
    // sprintf(passengers_serviced, "%d", passengersServiced();
    strcat(message, passengers_serviced);

    int len = strlen(message);
    if (copy_to_user(buf, message, len)) {
        printk(KERN_WARNING "copy_to_user failed\n");
        return -EFAULT;  // Return an error code indicating a copy failure
    }
    return len;
}

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file)
{
    kfree(message);
    return 0;
}

// /********************************************************************/

// static int elevator_init(void) {
// 	fops.proc_open = passenger_proc_open;
// 	// fops.proc_read = elevator_proc_read;
// 	fops.proc_release = elevator_proc_release;

// 	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
// 		printk(KERN_WARNING "elevator_init\n");
// 		remove_proc_entry(ENTRY_NAME, NULL);
// 		return -ENOMEM;
// 	}

// 	STUB_start_elevator = start_elevator;
//  STUB_issue_request = issue_request;
//  STUB_stop_elevator = stop_elevator;

// 	passengers.total_cnt = 0;
// 	passengers.total_weight = 0;

// 	INIT_LIST_HEAD(&passengers.list);
// 	INIT_LIST_HEAD(&f1.list);
// 	INIT_LIST_HEAD(&f2.list);
// 	INIT_LIST_HEAD(&f3.list);
// 	INIT_LIST_HEAD(&f4.list);
// 	INIT_LIST_HEAD(&f5.list);
// 	INIT_LIST_HEAD(&f6.list);
// 	return 0;
// }

// module_init(elevator_init);

// static void elevator_exit(void)
// {

//     STUB_start_elevator = NULL;
//     STUB_issue_request = NULL;
//     STUB_stop_elevator = NULL;

//     remove_proc_entry(ENTRY_NAME, PARENT);
// }

// module_exit(elevator_exit);
