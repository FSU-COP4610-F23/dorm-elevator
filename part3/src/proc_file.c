// # include "elevator.h"
// # include "elevator.c"

static struct proc_ops fops = {
    .proc_open = elevator_proc_open,
    .proc_read = elevator_proc_read,
    .proc_write = elevator_proc_write,
    .proc_release = elevator_proc_release,
};

static ssize_t elevator_proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    char buffer[10];
    if (count > 10) // We expect short commands
        return -EINVAL;

    if (copy_from_user(buffer, ubuf, count))
        return -EFAULT;

    buffer[min(count, sizeof(buffer) - 1)] = '\0'; // Ensure null-termination

    if (strcmp(buffer, "start\n") == 0)
    {
        return start_elevator();
    }
    else if (strcmp(buffer, "stop\n") == 0)
    {
        return stop_elevator();
    }
    else
    {
        return -EINVAL; // Invalid command
    }

    return count;
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
/*
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

    print_passengers();
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
*/

static ssize_t elevator_proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    // Buffer to hold the output
    char buf[10000];
    int len = 0; // Length of the output

    if (*ppos > 0) {
        return 0; // No more data to read
    }

    if (mutex_lock_interruptible(&elevator_mutex)) {
        return -ERESTARTSYS; // Return if the mutex acquisition was interrupted
    }

    // Construct the output
    // len += snprintf(buf + len, sizeof(buf) - len, "Elevator state: %s\n",
    //                 get_elevator_state_string(elevator_state));

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


    len += snprintf(buf + len, sizeof(buf) - len, "Current floor: %d\n", current_floor);
    len += snprintf(buf + len, sizeof(buf) - len, "Current load: %d lbs\n", elevator_weight);

    len += snprintf(buf + len, sizeof(buf) - len, "Elevator status: %s\n\n", get_elevator_status());

    // // Output the status of each floor
    // for (int i = 6; i >= 1; i--) {
    //     len += snprintf(buf + len, sizeof(buf) - len, "[%c] Floor %d: %s\n",
    //                     (current_floor == i) ? '*' : ' ', i, get_floor_status(i));
    // }

    // Include information about passengers in the elevator
    struct list_head *pos;
    Passenger *p;
    int i = 0;

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

    // Number of passengers, waiting and serviced
    len += snprintf(buf + len, sizeof(buf) - len, "\nNumber of passengers: %d\n", elevator.total_cnt);
    len += snprintf(buf + len, sizeof(buf) - len, "Number of passengers waiting: %d\n", FloorCountTotal());
    len += snprintf(buf + len, sizeof(buf) - len, "Number of passengers serviced: %d\n", passengers_serviced);

    mutex_unlock(&elevator_mutex); // Release the mutex

    // Copy the output to user space
    if (copy_to_user(ubuf, buf, len)) {
        return -EFAULT; // Copy to user failed
    }

    *ppos += len; // Update the reading position
    return len; // Return the number of characters read
}


char *get_elevator_status(void)
{
    static char status[256];
    int written = 0;
    int passenger_counts[NUM_PASSENGER_TYPES] = {0};

    // Assuming elevator.list is the list of passengers currently in the elevator
    struct list_head *pos;
    Passenger *p;

    // Count the number of each type of passenger in the elevator
    list_for_each(pos, &elevator.list)
    {
        p = list_entry(pos, Passenger, list);
        // Cast p->type to an unsigned int to use as an array index
        unsigned int type_index = (unsigned int)(p->type - '0'); // Assuming '0'-'3' are used for FRESHMAN-SOPHOMORE-JUNIOR-SENIOR
        if (type_index < NUM_PASSENGER_TYPES)
        {
            passenger_counts[type_index]++;
        }
    }

    // Now construct the status string based on the counts
    written += snprintf(status + written, sizeof(status) - written,
                        "F%d S%d J%d S%d",
                        passenger_counts[FRESHMAN],
                        passenger_counts[SOPHOMORE],
                        passenger_counts[JUNIOR],
                        passenger_counts[SENIOR]);

    // Ensure the string is null-terminated
    status[sizeof(status) - 1] = '\0';

    return status;
}

char *get_floor_status(int floor)
{
    static char floor_status[256];
    int written = 0;

    // Initialize the floor_status string to empty
    memset(floor_status, 0, sizeof(floor_status));

    // Iterate through the passenger list for the given floor
    struct list_head *pos;
    Passenger *p;
    int passenger_count = 0;

    list_for_each(pos, &floor_lists[floor - 1])
    {
        p = list_entry(pos, Passenger, list);
        // Write the passenger data to the floor_status string, checking buffer space
        if (written < sizeof(floor_status) - 1)
        {
            written += snprintf(floor_status + written, sizeof(floor_status) - written,
                                "%c%d ", p->type, p->destination);
            passenger_count++;
        }
    }

    // If no passengers were written, indicate that the floor is empty
    if (passenger_count == 0)
    {
        snprintf(floor_status, sizeof(floor_status), "0");
    }

    // Ensure the string is null-terminated
    floor_status[sizeof(floor_status) - 1] = '\0';

    return floor_status;
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
    int ret = 0;

    printk(KERN_INFO "Elevator module is being loaded\n");

    fops.proc_open = elevator_proc_open;
    fops.proc_read = elevator_proc_read;
    fops.proc_write = elevator_proc_write; // Make sure you have this line to handle writes
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

    elevator_state = OFFLINE; // Initialize elevator state to OFFLINE

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

    if (IS_ERR(elevator_thread))
    {
        printk(KERN_WARNING "elevator_init: Error creating elevator thread\n");
        ret = PTR_ERR(elevator_thread);
    }

    for (int i = 0; i < 6; i++)
    {
        INIT_LIST_HEAD(&floor_lists[i]);
    }

    message = kmalloc(ENTRY_SIZE, GFP_KERNEL);

    if (!message)
    {
        printk(KERN_WARNING "elevator_init: Error allocating message buffer\n");
        ret = -ENOMEM;
    }

    printk(KERN_INFO "Elevator module initialization completed\n");

    return 0;
}

module_init(elevator_init);

/*
static void __exit elevator_exit(void) {
    // Stop the elevator thread if it's running
    if (elevator_thread) {
        kthread_stop(elevator_thread);
        elevator_thread = NULL;  // Clear the thread pointer after stopping it
    }

    // If the elevator is active, stop it and perform necessary cleanup
    if (elevator_state != OFFLINE) {
        STUB_stop_elevator();
        wait_for_completion(&elevator_completion);
    }

    // Nullify the stubs after the elevator has been stopped
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    // Remove the proc entry
    remove_proc_entry(ENTRY_NAME, NULL);

    // Free any allocated memory for the message buffer
    if (message) {
        kfree(message);
        message = NULL;  // Clear the pointer after freeing
    }

    // Free any remaining passengers on the elevator
    struct list_head *temp, *pos;
    Passenger *p;

    mutex_lock(&elevator_mutex);  // Lock once to avoid locking and unlocking repeatedly
    list_for_each_safe(pos, temp, &elevator.list) {
        p = list_entry(pos, Passenger, list);
        list_del(pos);
        kfree(p);
    }
    // Free any remaining passengers on each floor
    for (int i = 0; i < 6; i++) {
        list_for_each_safe(pos, temp, &floor_lists[i]) {
            p = list_entry(pos, Passenger, list);
            list_del(pos);
            kfree(p);
        }
    }
    mutex_unlock(&elevator_mutex);  // Unlock after all passenger lists have been cleaned up

    printk(KERN_INFO "Elevator module unloaded\n");
}
*/

static void __exit elevator_exit(void)
{
    // Nullify the stubs to prevent them from being called while shutting down
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    // If the elevator is active, stop it and perform necessary cleanup
    if (elevator_state != OFFLINE)
    {
        stop_elevator();                           // Ensure this function signals the thread to exit and sets the state to OFFLINE
        wait_for_completion(&elevator_completion); // Wait for the elevator thread to finish
    }

    // Remove the proc entry
    remove_proc_entry(ENTRY_NAME, NULL);

    // Lock the mutex to ensure thread is not using the lists
    mutex_lock(&elevator_mutex);

    // Free any remaining passengers on the elevator and floors
    struct list_head *temp, *pos;
    Passenger *p;
    int i;

    // Cleanup passengers in the elevator
    list_for_each_safe(pos, temp, &elevator.list)
    {
        p = list_entry(pos, Passenger, list);
        list_del(pos);
        kfree(p);
    }

    // Cleanup passengers on each floor
    for (i = 0; i < 6; i++)
    {
        list_for_each_safe(pos, temp, &floor_lists[i])
        {
            p = list_entry(pos, Passenger, list);
            list_del(pos);
            kfree(p);
        }
    }

    // Unlock the mutex after cleaning up
    mutex_unlock(&elevator_mutex);

    // Stop the elevator thread if it's running
    if (elevator_thread)
    {
        kthread_stop(elevator_thread);
        elevator_thread = NULL; // Clear the thread pointer after stopping it
    }

    // Free any allocated memory for the message buffer
    if (message)
    {
        kfree(message);
        message = NULL; // Clear the pointer after freeing
    }

    printk(KERN_INFO "Elevator module unloaded\n");
}

module_exit(elevator_exit);