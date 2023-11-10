#include "elevator.h"
#include "proc_file.c"
// #include "passengers.c"

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

// Function to load passengers onto the elevator

void unload_passengers(int current_floor) {
    struct list_head *pos, *q;
    Passenger *passenger;

    list_for_each_safe(pos, q, &elevator.list) {
        passenger = list_entry(pos, Passenger, list);
        if (passenger->destination == current_floor) {
            list_del(pos); // Remove the passenger from the elevator list
            elevator.total_weight -= passenger->weight;
            elevator.total_cnt--;
            kfree(passenger); // If you have allocated memory for passengers, free it here
        }
    }
}


void load_passengers(int current_floor)
{
    struct list_head *floor_pos, *floor_temp;
    Passenger *floor_passenger;

    list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1])
    {
        floor_passenger = list_entry(floor_pos, Passenger, list);

        if (elevator.total_weight + floor_passenger->weight <= MAX_LOAD)
        {
            list_del(floor_pos);
            list_add_tail(floor_pos, &elevator.list);
            elevator.total_weight += floor_passenger->weight;
            elevator.total_cnt++;
        }
        else
        {
            break;
        }
    }
}

// Helper function to unload and load passengers
void unload_load_passengers(int current_floor)
{
    // Unload passengers first
    unload_passengers(current_floor);

    // Load new passengers
    load_passengers(current_floor);

}

// // Elevator thread function
// static int elevator_thread_function(void *data)
// {
//     while (!kthread_should_stop())
//     {
//         if (elevator_state == LOADING)
//         {
//             // Unload passengers with the same destination as the current floor
//             struct list_head *pos, *temp;
//             Passenger *p;

//             list_for_each_safe(pos, temp, &elevator.list)
//             {
//                 p = list_entry(pos, Passenger, list);
//                 if (p->destination == current_floor)
//                 {
//                     list_del(pos);
//                     // kfree(p);  // Since you didn't allocate memory for passengers, no need to free
//                     elevator.total_weight -= p->weight;
//                     elevator.total_cnt--;
//                 }
//             }

//             // Load passengers waiting on the current floor
//             load_passengers(current_floor);

//             if (list_empty(&floor_lists[current_floor - 1]))
//             {
//                 elevator_state = IDLE;
//             }
//             else
//             {
//                 elevator_state = LOADING;
//             }
//         }
//         else if (elevator_state == IDLE)
//         {
//             // Check if there are any passengers in the elevator
//             if (list_empty(&elevator.list))
//             {
//                 // If the elevator is empty, stay in IDLE state
//                 elevator_state = IDLE;
//             }
//             else
//             {
//                 // If there are passengers in the elevator, unload passengers at the current floor
//                 struct list_head *pos, *temp;
//                 Passenger *p;

//                 list_for_each_safe(pos, temp, &elevator.list)
//                 {
//                     p = list_entry(pos, Passenger, list);
//                     if (p->destination == current_floor)
//                     {
//                         list_del(pos);
//                         // kfree(p);  // Since you didn't allocate memory for passengers, no need to free
//                         elevator.total_weight -= p->weight;
//                         elevator.total_cnt--;
//                     }
//                 }

//                 // Load passengers waiting on the current floor
//                 load_passengers(current_floor);

//                 if (!list_empty(&floor_lists[current_floor - 1]))
//                 {
//                     elevator_state = LOADING;
//                 }
//                 else
//                 {
//                     elevator_state = IDLE;
//                 }
//             }

//             msleep(1000); // Sleep for 1 second in IDLE state
//         }
//         else if (elevator_state == UP || elevator_state == DOWN)
//         {
//             // Check if the elevator has reached its destination floor
//             if (current_floor == 1 || current_floor == 6)
//             {
//                 // The elevator has reached the top or bottom floor, change direction
//                 if (elevator_state == UP)
//                 {
//                     elevator_state = DOWN;
//                 }
//                 else
//                 {
//                     elevator_state = UP;
//                 }
//             }
//             else
//             {
//                 // Move to the next floor
//                 current_floor = (elevator_state == UP) ? current_floor + 1 : current_floor - 1;

//                 // Unload passengers with the same destination as the current floor
//                 struct list_head *pos, *temp;
//                 Passenger *p;

//                 list_for_each_safe(pos, temp, &elevator.list)
//                 {
//                     p = list_entry(pos, Passenger, list);
//                     if (p->destination == current_floor)
//                     {
//                         list_del(pos);
//                         // kfree(p);  // Since you didn't allocate memory for passengers, no need to free
//                         elevator.total_weight -= p->weight;
//                         elevator.total_cnt--;
//                     }
//                 }

//                 // Load passengers waiting on the current floor
//                 load_passengers(current_floor);

//                 msleep(1000); // Sleep for 1 second between floors
//             }
//         }

//         msleep(1000); // Sleep for 1 second between iterations
//     }

//     return 0;
// }

// Elevator thread function

int decide_next_move(void)
{
    int i;
    int passengers_above = 0;
    int passengers_below = 0;

    // Check for passengers waiting above the current floor
    for (i = current_floor; i < 6; i++)
    {
        if (!list_empty(&floor_lists[i]))
        {
            passengers_above = 1;
            break;
        }
    }

    // Check for passengers waiting below the current floor
    for (i = current_floor - 2; i >= 0; i--)
    { // array index is floor - 1
        if (!list_empty(&floor_lists[i]))
        {
            passengers_below = 1;
            break;
        }
    }

    // Decide the next move based on passenger location
    // If there are passengers above and none below, or passengers on both sides,
    // we choose to go up. If there are only passengers below, we go down.
    if (passengers_above && !passengers_below)
    {
        return UP;
    }
    else if (passengers_below)
    {
        return DOWN;
    }
    else
    {
        // If there are no passengers waiting, you might choose to stay idle
        // or return to a home floor.
        return IDLE;
    }
}

// Helper function to check if there are passengers waiting on any floor
int passengers_waiting_anywhere(void)
{

    for (int i = 0; i < 6; i++)
    { // Assuming TOP_FLOOR is the total number of floors
        if (!list_empty(&floor_lists[i]))
        {             // If any list is not empty, passengers are waiting
            return 1; // Passengers are waiting somewhere
        }
    }

    return 0; // No passengers waiting on any floor
}


static int elevator_thread_function(void *data)
{
    while (!kthread_should_stop())
    {
        if (mutex_lock_interruptible(&elevator_mutex) == 0)
        {
            if (elevator_state == LOADING)
            {
                unload_load_passengers(current_floor); // Unload and load passengers at the current floor
                elevator_state = decide_next_move();   // Decide the next state of the elevator
            }
            else if (elevator_state == IDLE)
            {
                if (passengers_waiting_anywhere())
                {
                    elevator_state = UP; // Start moving up
                }
            }
            else if (elevator_state == UP)
            {
                if (current_floor < 6)
                {
                    current_floor++;
                    unload_load_passengers(current_floor); // Unload and load passengers at the current floor
                }
                if (current_floor == 6)
                {
                    elevator_state = DOWN; // Change direction at the top floor
                }
            }
            else if (elevator_state == DOWN)
            {
                if (current_floor > 1)
                {
                    current_floor--;
                    unload_load_passengers(current_floor); // Unload and load passengers at the current floor
                }
                if (current_floor == 1)
                {
                    elevator_state = UP; // Change direction at the bottom floor
                }
            }
            mutex_unlock(&elevator_mutex);
            msleep(FLOOR_TRAVEL_TIME); // Sleep to simulate floor travel time
        }
        else
        {
            // Handle the error if mutex lock was interrupted
            printk(KERN_ALERT "Elevator thread interrupted while trying to lock the mutex\n");
        }
        msleep(LOOP_SLEEP_TIME); // Sleep between loop iterations if needed
    }
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

/*
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
*/

int stop_elevator(void)
{
    int ret = 0; // Default return value indicating success

    // Acquire the mutex to ensure exclusive access to shared resources
    if (mutex_lock_interruptible(&elevator_mutex))
    {
        printk(KERN_INFO "stop_elevator: Could not lock mutex.\n");
        return -ERESTARTSYS; // Return an error if mutex acquisition was interrupted
    }

    // Check if the elevator is already offline
    if (elevator_state == OFFLINE)
    {
        printk(KERN_INFO "stop_elevator: Elevator is already offline.\n");
        ret = 1; // Elevator is already deactivating, so return 1
    }
    else if (list_empty(&elevator.list) && passengers_serviced == elevatorCount())
    {
        printk(KERN_INFO "stop_elevator: Stopping elevator, no passengers and all serviced.\n");
        elevator_state = OFFLINE; // Transition to OFFLINE state
        // Signal the elevator thread to stop if necessary
        // Note: You would need to implement signaling logic based on your thread management
    }
    else
    {
        printk(KERN_INFO "stop_elevator: Elevator cannot stop, passengers present or not all serviced.\n");
        ret = 1; // Elevator is not empty or not all passengers have been serviced, return 1
    }

    // Release the mutex after finished modifying shared resources
    mutex_unlock(&elevator_mutex);

    // If the elevator was successfully stopped, you may need additional logic to clean up
    // any remaining resources, notify userspace, etc.
    if (ret == 0)
    {
        // Perform necessary cleanup
        printk(KERN_INFO "stop_elevator: Cleanup after stopping elevator.\n");
        // Example: complete(&elevator_completion); // Signal the elevator thread to exit
    }

    return ret; // Return the result of the stop attempt
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
