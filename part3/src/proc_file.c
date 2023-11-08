// # include "elevator.h"
// # include "elevator.c"


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

    for (int i = 0; i < 6; i++)
    {
        INIT_LIST_HEAD(&floor_lists[i]);
    }

    message = kmalloc(ENTRY_SIZE, GFP_KERNEL);

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