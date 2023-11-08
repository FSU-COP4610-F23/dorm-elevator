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

int issue_request(int start_floor, int destination_floor, int type)
{
    // Check if the type is valid
    if (type < 0 || type >= NUM_PASSENGER_TYPES)
    {
        return 1; // Return 1 for an invalid type
    }

    // Check if the start floor is out of range
    if (start_floor < 1 || start_floor > 6)
    {
        return 1; // Return 1 for an invalid start floor
    }

    // Create a new passenger
    Passenger *p = kmalloc(sizeof(Passenger), GFP_KERNEL);

    if (p == NULL)
    {
        return 1; // Return 1 for memory allocation failure
    }

    // Define passenger type weights and IDs in arrays
    int type_weights[NUM_PASSENGER_TYPES] = {0, 1, 2, 3};
    char type_ids[NUM_PASSENGER_TYPES][2] = {"F", "O", "J", "S"};

    int weight = type_weights[type];
    char id[3] = "";
    sprintf(id, "%c%d", type_ids[type][0], destination_floor);

    // Initialize passenger data
    sprintf(p->id, "%s", id);
    p->weight = weight;
    p->destination = destination_floor;

    // Add the passenger to the appropriate floor's waiting list
    list_add_tail(&p->list, &floor_lists[start_floor - 1]);
    floor_count[start_floor - 1]++;

    return 0; // Return 0 for a valid request
}

int print_passengers(void)
{
    // ... (unchanged)
    struct list_head *temp;
    Passenger *p;
    char *buf = kmalloc(100, GFP_KERNEL); // Allocate a buffer for formatting

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

    // Elevator status
    sprintf(buf, "Elevator status: ");
    switch (elevator_state)
    {
    case OFFLINE:
        strcat(buf, "OFFLINE");
        break;
    case IDLE:
        strcat(buf, "IDLE");
        break;
    case LOADING:
        strcat(buf, "LOADING");
        break;
    case UP:
        strcat(buf, "UP");
        break;
    case DOWN:
        strcat(buf, "DOWN");
        break;
    }
    strcat(buf, "\n");
    strcat(message, buf);

    // Add current floor and elevator load
    sprintf(buf, "Current floor: %d\n", current_floor);
    strcat(message, buf);

    sprintf(buf, "Current load: %d lbs\n", elevator_weight);
    strcat(message, buf);

    // Iterate through the elevator's passenger list
    sprintf(buf, "Elevator status:\n");
    strcat(message, buf);

    int i = 0;
    list_for_each(temp, &elevator.list)
    {
        p = list_entry(temp, Passenger, list);

        if (i % 5 == 0 && i > 0)
        {
            strcat(message, "\n");
        }

        // Format passenger information and add it to the message
        // e.g., "F1 O3 S5"
        sprintf(buf, "%s%d ", p->id, p->destination);
        strcat(message, buf);

        i++;
    }

    strcat(message, "\n");

    kfree(buf);
    return 0;
}

int delete_passengers(int type)
{
    // ... (unchanged)
    // Check if the type is valid
    if (type < 0 || type >= NUM_PASSENGER_TYPES)
    {
        return 1; // Return 1 for an invalid type
    }

    // Use kmalloc to allocate a buffer for formatting the type
    char type_str[4]; // Adjust the size as needed
    snprintf(type_str, sizeof(type_str), "%c", "FOSJ"[type]);

    struct list_head *temp, *pos;
    Passenger *p;

    // Lock the elevator_mutex to access shared data
    mutex_lock(&elevator_mutex);

    // Delete passengers of the specified type from the elevator
    list_for_each_safe(pos, temp, &elevator.list)
    {
        p = list_entry(pos, Passenger, list);

        if (p->id[0] == type_str[0])
        {
            elevator.total_cnt--;
            elevator.total_weight -= p->weight;
            list_del(pos);
            kfree(p);
        }
    }

    // Unlock the elevator_mutex
    mutex_unlock(&elevator_mutex);

    // Lock the elevator_mutex for each floor and remove passengers from waiting lists
    for (int i = 0; i < 6; i++)
    {
        if (i == type)
        {
            continue; // Skip the specified floor
        }

        mutex_lock(&elevator_mutex);

        // Delete passengers of the specified type from the floor's waiting list
        list_for_each_safe(pos, temp, &floor_lists[i])
        {
            p = list_entry(pos, Passenger, list);

            if (p->id[0] == type_str[0])
            {
                floor_count[i]--;
                list_del(pos);
                kfree(p);
            }
        }

        mutex_unlock(&elevator_mutex);
    }

    return 0;
}

static int elevator_thread_function(void *data)
{
    struct Elevator *elevator = (struct Elevator *)data;

    while (!kthread_should_stop())
    {
        set_current_state(TASK_INTERRUPTIBLE);

        if (elevator_state == LOADING)
        {
            struct list_head *pos, *temp;
            Passenger *p;

            mutex_lock(&elevator_mutex);

            list_for_each_safe(pos, temp, &elevator->list)
            {
                p = list_entry(pos, Passenger, list);
                if (p->destination == current_floor)
                {
                    list_del(pos);
                    kfree(p);
                    elevator->total_weight -= p->weight;
                    elevator->total_cnt--;
                    elevator_weight -= p->weight; // Update elevator_weight
                    passengers_serviced++;
                }
            }

            // Load passengers waiting on the current floor
            struct list_head *floor_pos, *floor_temp;
            Passenger *floor_passenger;

            list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1])
            {
                floor_passenger = list_entry(floor_pos, Passenger, list);

                if (elevator_weight + floor_passenger->weight <= MAX_LOAD)
                {
                    list_del(floor_pos);
                    list_add_tail(floor_pos, &elevator->list);
                    elevator_weight += floor_passenger->weight;
                    elevator->total_weight += floor_passenger->weight;
                    elevator->total_cnt++;
                }
                else
                {
                    break;
                }
            }

            mutex_unlock(&elevator_mutex);

            if (list_empty(&floor_lists[current_floor - 1]))
            {
                elevator_state = IDLE;
            }
            else
            {
                elevator_state = LOADING;
            }
        }
        else if (elevator_state == IDLE)
        {
            // Logic for IDLE state
            set_current_state(TASK_INTERRUPTIBLE);

            // Check if there are any passengers in the elevator
            if (list_empty(&elevator->list))
            {
                // If the elevator is empty, stay in IDLE state
                elevator_state = IDLE;
            }
            else
            {
                // If there are passengers in the elevator, unload passengers at the current floor
                struct list_head *pos, *temp;
                Passenger *p;

                mutex_lock(&elevator_mutex);

                list_for_each_safe(pos, temp, &elevator->list)
                {
                    p = list_entry(pos, Passenger, list);
                    if (p->destination == current_floor)
                    {
                        list_del(pos);
                        kfree(p);
                        elevator->total_weight -= p->weight;
                        elevator->total_cnt--;
                        elevator_weight -= p->weight;
                        passengers_serviced++;
                    }
                }

                mutex_unlock(&elevator_mutex);

                // Check if there are passengers waiting on the current floor
                if (!list_empty(&floor_lists[current_floor - 1]))
                {
                    // Load passengers from the current floor
                    mutex_lock(&elevator_mutex);

                    struct list_head *floor_pos, *floor_temp;
                    Passenger *floor_passenger;

                    list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1])
                    {
                        floor_passenger = list_entry(floor_pos, Passenger, list);

                        if (elevator_weight + floor_passenger->weight <= MAX_LOAD)
                        {
                            list_del(floor_pos);
                            list_add_tail(floor_pos, &elevator->list);
                            elevator_weight += floor_passenger->weight;
                            elevator->total_weight += floor_passenger->weight;
                            elevator->total_cnt++;
                        }
                        else
                        {
                            break;
                        }
                    }

                    mutex_unlock(&elevator_mutex);

                    elevator_state = LOADING;
                }
                else
                {
                    // If no passengers are waiting on the current floor, stay in IDLE state
                    elevator_state = IDLE;
                }
            }

            schedule_timeout(HZ);
        }
        else if (elevator_state == UP || elevator_state == DOWN)
        {
            // Logic for UP or DOWN state
            set_current_state(TASK_INTERRUPTIBLE);

            // Check if the elevator is moving up or down
            int next_floor;
            if (elevator_state == UP)
            {
                next_floor = current_floor + 1;
            }
            else // elevator_state == DOWN
            {
                next_floor = current_floor - 1;
            }

            // Check if the elevator has reached its destination floor
            if (next_floor == 0 || next_floor == 7)
            {
                // The elevator has reached the top or bottom floor, change direction
                if (elevator_state == UP)
                {
                    elevator_state = DOWN;
                }
                else // elevator_state == DOWN
                {
                    elevator_state = UP;
                }
            }
            else
            {
                // Update the elevator's current floor
                current_floor = next_floor;

                // Unload passengers with the same destination as the current floor
                struct list_head *pos, *temp;
                Passenger *p;

                mutex_lock(&elevator_mutex);

                list_for_each_safe(pos, temp, &elevator->list)
                {
                    p = list_entry(pos, Passenger, list);
                    if (p->destination == current_floor)
                    {
                        list_del(pos);
                        kfree(p);
                        elevator->total_weight -= p->weight;
                        elevator->total_cnt--;
                        elevator_weight -= p->weight;
                        passengers_serviced++;
                    }
                }

                mutex_unlock(&elevator_mutex);

                // Check if there are passengers waiting on the current floor
                if (!list_empty(&floor_lists[current_floor - 1]))
                {
                    // Load passengers from the current floor
                    mutex_lock(&elevator_mutex);

                    struct list_head *floor_pos, *floor_temp;
                    Passenger *floor_passenger;

                    list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1])
                    {
                        floor_passenger = list_entry(floor_pos, Passenger, list);

                        if (elevator_weight + floor_passenger->weight <= MAX_LOAD)
                        {
                            list_del(floor_pos);
                            list_add_tail(floor_pos, &elevator->list);
                            elevator_weight += floor_passenger->weight;
                            elevator->total_weight += floor_passenger->weight;
                            elevator->total_cnt++;
                        }
                        else
                        {
                            break;
                        }
                    }

                    mutex_unlock(&elevator_mutex);

                    elevator_state = LOADING;
                }
                else
                {
                    // If no passengers are waiting on the current floor, stay in UP or DOWN state
                    // and continue moving to the next floor
                }
            }

            schedule_timeout(HZ);
        }

        schedule_timeout(HZ);
    }

    complete(&elevator_completion);
    return 0;
}

int start_elevator(void)
{
    // Check if the elevator is already active
    if (elevator_state != OFFLINE)
    {
        return 1;
    }

    // Initialize the elevator's state and other parameters
    elevator_state = IDLE;
    current_floor = 1;
    elevator_weight = 0;
    passengers_serviced = 0;

    // Check if memory allocation fails
    message = kmalloc(sizeof(char) * ENTRY_SIZE, GFP_KERNEL);
    if (message == NULL)
    {
        printk(KERN_WARNING "start_elevator: Failed to allocate memory\n");
        return -ENOMEM;
    }

    // Start the elevator thread
    elevator_thread = kthread_run(elevator_thread_function, &elevator, "elevator_thread");
    if (elevator_thread == NULL)
    {
        printk(KERN_WARNING "start_elevator: Failed to create elevator_thread\n");
        kfree(message); // Clean up allocated memory
        return -ERRORNUM;
    }

    return 0; // Return 0 for a successful start
}

int elevatorCount(void)
{
    int count = 0;
    struct list_head *pos;
    Passenger *p;

    mutex_lock(&elevator_mutex);

    list_for_each(pos, &elevator.list)
    {
        p = list_entry(pos, Passenger, list);
        count++;
    }

    mutex_unlock(&elevator_mutex);

    return count;
}

int stop_elevator(void)
{
    // Check if the elevator is already in the process of deactivating
    if (elevator_state == OFFLINE)
    {
        return 1; // Return 1 if the elevator is already deactivating
    }

    // Lock the elevator_mutex to ensure exclusive access
    mutex_lock(&elevator_mutex);

    if (list_empty(&elevator.list) && passengers_serviced == elevatorCount())
    {
        // If the elevator is empty and all passengers have been serviced
        elevator_state = OFFLINE;      // Transition to OFFLINE state
        mutex_unlock(&elevator_mutex); // Unlock the elevator_mutex
        return 0;                      // Return 0 for successful deactivation
    }

    mutex_unlock(&elevator_mutex); // Unlock the elevator_mutex

    return 1; // Return 1 if the elevator is not empty and deactivation is in progress
}

int FloorCountTotal(void)
{
    int total_count = 0;

    // Calculate the total number of passengers waiting on all floors
    for (int i = 0; i < 6; i++)
    {
        total_count += floor_count[i];
    }

    return total_count;
}

int passengersServiced(void)
{
    return passengers_serviced;
}

int elevator_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    read_p = 1;

    // Allocate memory for the message buffer
    message = kmalloc(ENTRY_SIZE, GFP_KERNEL);

    if (message == NULL)
    {
        printk(KERN_WARNING "elevator_proc_open: Failed to allocate memory\n");
        return -ENOMEM;
    }

    // Clear the message buffer
    memset(message, 0, ENTRY_SIZE);

    // Prepare and format the elevator status and information
    char elevator_status[20];

    // Determine the elevator state and set the elevator_status accordingly
    switch (elevator_state)
    {
    case OFFLINE:
        strcpy(elevator_status, "OFFLINE");
        break;
    case IDLE:
        strcpy(elevator_status, "IDLE");
        break;
    case LOADING:
        strcpy(elevator_status, "LOADING");
        break;
    case UP:
        strcpy(elevator_status, "UP");
        break;
    case DOWN:
        strcpy(elevator_status, "DOWN");
        break;
    default:
        strcpy(elevator_status, "UNKNOWN");
        break;
    }

    // Add elevator information to the message
    snprintf(message, ENTRY_SIZE, "Elevator state: %s\n", elevator_status);
    snprintf(message + strlen(message), ENTRY_SIZE - strlen(message), "Current floor: %d\n", current_floor);
    snprintf(message + strlen(message), ENTRY_SIZE - strlen(message), "Current load: %d lbs\n", elevator_weight);

    // Add information about passengers in the elevator
    print_passengers();

    // Add information about passengers waiting on each floor
    for (int floor = 1; floor <= 6; floor++)
    {
        snprintf(message + strlen(message), ENTRY_SIZE - strlen(message), "[%c] Floor %d: %d",
                 (current_floor == floor) ? '*' : ' ', floor, floor_count[floor - 1]);

        // Add information about waiting passengers on this floor
        struct list_head *pos;
        Passenger *p;
        list_for_each(pos, &floor_lists[floor - 1])
        {
            p = list_entry(pos, Passenger, list);
            snprintf(message + strlen(message), ENTRY_SIZE - strlen(message), " %c%d", p->id[0], p->destination);
        }

        strcat(message, "\n");
    }

    // Add the total number of passengers and passengers serviced
    snprintf(message + strlen(message), ENTRY_SIZE - strlen(message), "Number of passengers: %d\n", elevatorCount());
    snprintf(message + strlen(message), ENTRY_SIZE - strlen(message), "Number of passengers waiting: %d\n", FloorCountTotal());
    snprintf(message + strlen(message), ENTRY_SIZE - strlen(message), "Number of passengers serviced: %d\n", passengersServiced());

    return 0;
}

static ssize_t elevator_proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[10000];
    int len = 0;

    // Include the actual elevator state, current floor, current load, and elevator status
    len = sprintf(buf, "Elevator state: ");
    switch (elevator_state)
    {
    case OFFLINE:
        len += sprintf(buf + len, "OFFLINE\n");
        break;
    case IDLE:
        len += sprintf(buf + len, "IDLE\n");
        break;
    case LOADING:
        len += sprintf(buf + len, "LOADING\n");
        break;
    case UP:
        len += sprintf(buf + len, "UP\n");
        break;
    case DOWN:
        len += sprintf(buf + len, "DOWN\n");
        break;
    default:
        len += sprintf(buf + len, "UNKNOWN\n");
        break;
    }

    len += sprintf(buf + len, "Current floor: %d\n", current_floor);
    len += sprintf(buf + len, "Current load: %d lbs\n", elevator_weight);

    // Include information about passengers in the elevator
    struct list_head *pos;
    Passenger *p;
    int i = 0;
    list_for_each(pos, &elevator.list)
    {
        p = list_entry(pos, Passenger, list);
        if (i % 5 == 0 && i > 0)
        {
            len += sprintf(buf + len, "\n");
        }
        len += sprintf(buf + len, "%s%d ", p->id, p->destination);
        i++;
    }
    len += sprintf(buf + len, "\n");

    // Include information about passengers waiting on each floor in reverse order
    for (int floor = 6; floor >= 1; floor--)
    {
        len += sprintf(buf + len, "[%c] Floor %d: %d",
                       (current_floor == floor) ? '*' : ' ', floor, floor_count[floor - 1]);

        struct list_head *floor_pos;
        Passenger *floor_passenger;
        list_for_each(pos, &floor_lists[floor - 1])
        {
            floor_passenger = list_entry(pos, Passenger, list);
            len += sprintf(buf + len, " %c%d", floor_passenger->id[0], floor_passenger->destination);
        }
        len += sprintf(buf + len, "\n");
    }

    // Include the total number of passengers and passengers serviced
    len += sprintf(buf + len, "Number of passengers: %d\n", elevatorCount());
    len += sprintf(buf + len, "Number of passengers waiting: %d\n", FloorCountTotal());
    len += sprintf(buf + len, "Number of passengers serviced: %d\n", passengersServiced());

    return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file)
{
    if (message)
    {
        kfree(message);
        message = NULL;
    }
    return 0;
}

static int __init elevator_init(void)
{
    printk(KERN_INFO "Elevator module is being loaded\n");

    fops.proc_open = elevator_proc_open;
    fops.proc_read = elevator_proc_read;
    fops.proc_release = elevator_proc_release;

    if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
    {
        printk(KERN_WARNING "elevator_init\n");
        remove_proc_entry(ENTRY_NAME, NULL);
        return -ENOMEM;
    }

    // Initialize elevator state, lists, mutex, and start elevator thread
    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;

    elevator.total_cnt = 0;
    elevator.total_weight = 0;

    INIT_LIST_HEAD(&elevator.list);
    INIT_LIST_HEAD(&f1.list);
    INIT_LIST_HEAD(&f2.list);
    INIT_LIST_HEAD(&f3.list);
    INIT_LIST_HEAD(&f4.list);
    INIT_LIST_HEAD(&f5.list);
    INIT_LIST_HEAD(&f6.list);

    mutex_init(&elevator_mutex);

    elevator_thread = kthread_run(elevator_thread_function, &elevator, "elevator_thread");

    for (int i = 0; i < 6; i++)
    {
        INIT_LIST_HEAD(&floor_lists[i]);
    }

    printk(KERN_INFO "Elevator module initialization completed\n");
    return 0;
}

module_init(elevator_init);

static void __exit elevator_exit(void)
{
    if (elevator_state != OFFLINE)
    {
        // If the elevator is active, stop it and perform necessary cleanup
        STUB_stop_elevator();

        // Wait for the elevator thread to finish
        wait_for_completion(&elevator_completion);
    }

    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    remove_proc_entry(ENTRY_NAME, PARENT);

    // Stop the elevator thread if it's running
    if (elevator_thread)
    {
        kthread_stop(elevator_thread);
    }

    // Free any allocated memory
    if (message)
    {
        kfree(message);
    }

    // Free any remaining passengers
    struct list_head *temp, *pos;
    Passenger *p;

    mutex_lock(&elevator_mutex);
    list_for_each_safe(pos, temp, &elevator.list)
    {
        p = list_entry(pos, Passenger, list);
        list_del(pos);
        kfree(p);
    }
    mutex_unlock(&elevator_mutex);

    for (int i = 0; i < 6; i++)
    {
        mutex_lock(&elevator_mutex);
        list_for_each_safe(pos, temp, &floor_lists[i])
        {
            p = list_entry(pos, Passenger, list);
            list_del(pos);
            kfree(p);
        }
        mutex_unlock(&elevator_mutex);
    }
}

module_exit(elevator_exit);
