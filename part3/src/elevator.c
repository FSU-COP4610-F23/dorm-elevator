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
void load_passengers(int current_floor) {
    struct list_head *floor_pos, *floor_temp;
    Passenger *floor_passenger;

    list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1]) {
        floor_passenger = list_entry(floor_pos, Passenger, list);

        if (elevator.total_weight + floor_passenger->weight <= MAX_LOAD) {
            list_del(floor_pos);
            list_add_tail(floor_pos, &elevator.list);
            elevator.total_weight += floor_passenger->weight;
            elevator.total_cnt++;
        } else {
            break;
        }
    }
}

// Elevator thread function
static int elevator_thread_function(void *data) {
    while (!kthread_should_stop()) {
        if (elevator_state == LOADING) {
            // Unload passengers with the same destination as the current floor
            struct list_head *pos, *temp;
            Passenger *p;

            list_for_each_safe(pos, temp, &elevator.list) {
                p = list_entry(pos, Passenger, list);
                if (p->destination == current_floor) {
                    list_del(pos);
                    //kfree(p);  // Since you didn't allocate memory for passengers, no need to free
                    elevator.total_weight -= p->weight;
                    elevator.total_cnt--;
                }
            }

            // Load passengers waiting on the current floor
            load_passengers(current_floor);

            if (list_empty(&floor_lists[current_floor - 1])) {
                elevator_state = IDLE;
            } else {
                elevator_state = LOADING;
            }
        } else if (elevator_state == IDLE) {
            // Check if there are any passengers in the elevator
            if (list_empty(&elevator.list)) {
                // If the elevator is empty, stay in IDLE state
                elevator_state = IDLE;
            } else {
                // If there are passengers in the elevator, unload passengers at the current floor
                struct list_head *pos, *temp;
                Passenger *p;

                list_for_each_safe(pos, temp, &elevator.list) {
                    p = list_entry(pos, Passenger, list);
                    if (p->destination == current_floor) {
                        list_del(pos);
                        //kfree(p);  // Since you didn't allocate memory for passengers, no need to free
                        elevator.total_weight -= p->weight;
                        elevator.total_cnt--;
                    }
                }

                // Load passengers waiting on the current floor
                load_passengers(current_floor);

                if (!list_empty(&floor_lists[current_floor - 1])) {
                    elevator_state = LOADING;
                } else {
                    elevator_state = IDLE;
                }
            }

            msleep(1000);  // Sleep for 1 second in IDLE state
        } else if (elevator_state == UP || elevator_state == DOWN) {
            // Check if the elevator has reached its destination floor
            if (current_floor == 1 || current_floor == 6) {
                // The elevator has reached the top or bottom floor, change direction
                if (elevator_state == UP) {
                    elevator_state = DOWN;
                } else {
                    elevator_state = UP;
                }
            } else {
                // Move to the next floor
                current_floor = (elevator_state == UP) ? current_floor + 1 : current_floor - 1;

                // Unload passengers with the same destination as the current floor
                struct list_head *pos, *temp;
                Passenger *p;

                list_for_each_safe(pos, temp, &elevator.list) {
                    p = list_entry(pos, Passenger, list);
                    if (p->destination == current_floor) {
                        list_del(pos);
                        //kfree(p);  // Since you didn't allocate memory for passengers, no need to free
                        elevator.total_weight -= p->weight;
                        elevator.total_cnt--;
                    }
                }

                // Load passengers waiting on the current floor
                load_passengers(current_floor);

                msleep(1000);  // Sleep for 1 second between floors
            }
        }

        msleep(1000);  // Sleep for 1 second between iterations
    }

    return 0;
}








// static int elevator_thread_function(void *data)
// {
//     struct Elevator *elevator = (struct Elevator *)data;

//     while (!kthread_should_stop())
//     {
//         set_current_state(TASK_INTERRUPTIBLE);

//         if (elevator_state == LOADING)
//         {
//             struct list_head *pos, *temp;
//             Passenger *p;

//             mutex_lock(&elevator_mutex);

//             list_for_each_safe(pos, temp, &elevator->list)
//             {
//                 p = list_entry(pos, Passenger, list);
//                 if (p->destination == current_floor)
//                 {
//                     list_del(pos);
//                     kfree(p);
//                     elevator->total_weight -= p->weight;
//                     elevator->total_cnt--;
//                     elevator_weight -= p->weight; // Update elevator_weight
//                     passengers_serviced++;
//                 }
//             }

//             // Load passengers waiting on the current floor
//             struct list_head *floor_pos, *floor_temp;
//             Passenger *floor_passenger;

//             list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1])
//             {
//                 floor_passenger = list_entry(floor_pos, Passenger, list);

//                 if (elevator_weight + floor_passenger->weight <= MAX_LOAD)
//                 {
//                     list_del(floor_pos);
//                     list_add_tail(floor_pos, &elevator->list);
//                     elevator_weight += floor_passenger->weight;
//                     elevator->total_weight += floor_passenger->weight;
//                     elevator->total_cnt++;
//                 }
//                 else
//                 {
//                     break;
//                 }
//             }

//             mutex_unlock(&elevator_mutex);

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
//             // Logic for IDLE state
//             set_current_state(TASK_INTERRUPTIBLE);

//             // Check if there are any passengers in the elevator
//             if (list_empty(&elevator->list))
//             {
//                 // If the elevator is empty, stay in IDLE state
//                 elevator_state = IDLE;
//             }
//             else
//             {
//                 // If there are passengers in the elevator, unload passengers at the current floor
//                 struct list_head *pos, *temp;
//                 Passenger *p;

//                 mutex_lock(&elevator_mutex);

//                 list_for_each_safe(pos, temp, &elevator->list)
//                 {
//                     p = list_entry(pos, Passenger, list);
//                     if (p->destination == current_floor)
//                     {
//                         list_del(pos);
//                         kfree(p);
//                         elevator->total_weight -= p->weight;
//                         elevator->total_cnt--;
//                         elevator_weight -= p->weight;
//                         passengers_serviced++;
//                     }
//                 }

//                 mutex_unlock(&elevator_mutex);

//                 // Check if there are passengers waiting on the current floor
//                 if (!list_empty(&floor_lists[current_floor - 1]))
//                 {
//                     // Load passengers from the current floor
//                     mutex_lock(&elevator_mutex);

//                     struct list_head *floor_pos, *floor_temp;
//                     Passenger *floor_passenger;

//                     list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1])
//                     {
//                         floor_passenger = list_entry(floor_pos, Passenger, list);

//                         if (elevator_weight + floor_passenger->weight <= MAX_LOAD)
//                         {
//                             list_del(floor_pos);
//                             list_add_tail(floor_pos, &elevator->list);
//                             elevator_weight += floor_passenger->weight;
//                             elevator->total_weight += floor_passenger->weight;
//                             elevator->total_cnt++;
//                         }
//                         else
//                         {
//                             break;
//                         }
//                     }

//                     mutex_unlock(&elevator_mutex);

//                     elevator_state = LOADING;
//                 }
//                 else
//                 {
//                     // If no passengers are waiting on the current floor, stay in IDLE state
//                     elevator_state = IDLE;
//                 }
//             }

//             schedule_timeout(HZ);
//         }
//         else if (elevator_state == UP || elevator_state == DOWN)
//         {
//             // Logic for UP or DOWN state
//             set_current_state(TASK_INTERRUPTIBLE);

//             // Check if the elevator is moving up or down
//             int next_floor;
//             if (elevator_state == UP)
//             {
//                 next_floor = current_floor + 1;
//             }
//             else // elevator_state == DOWN
//             {
//                 next_floor = current_floor - 1;
//             }

//             // Check if the elevator has reached its destination floor
//             if (next_floor == 0 || next_floor == 7)
//             {
//                 // The elevator has reached the top or bottom floor, change direction
//                 if (elevator_state == UP)
//                 {
//                     elevator_state = DOWN;
//                 }
//                 else // elevator_state == DOWN
//                 {
//                     elevator_state = UP;
//                 }
//             }
//             else
//             {
//                 // Update the elevator's current floor
//                 current_floor = next_floor;

//                 // Unload passengers with the same destination as the current floor
//                 struct list_head *pos, *temp;
//                 Passenger *p;

//                 mutex_lock(&elevator_mutex);

//                 list_for_each_safe(pos, temp, &elevator->list)
//                 {
//                     p = list_entry(pos, Passenger, list);
//                     if (p->destination == current_floor)
//                     {
//                         list_del(pos);
//                         kfree(p);
//                         elevator->total_weight -= p->weight;
//                         elevator->total_cnt--;
//                         elevator_weight -= p->weight;
//                         passengers_serviced++;
//                     }
//                 }

//                 mutex_unlock(&elevator_mutex);

//                 // Check if there are passengers waiting on the current floor
//                 if (!list_empty(&floor_lists[current_floor - 1]))
//                 {
//                     // Load passengers from the current floor
//                     mutex_lock(&elevator_mutex);

//                     struct list_head *floor_pos, *floor_temp;
//                     Passenger *floor_passenger;

//                     list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1])
//                     {
//                         floor_passenger = list_entry(floor_pos, Passenger, list);

//                         if (elevator_weight + floor_passenger->weight <= MAX_LOAD)
//                         {
//                             list_del(floor_pos);
//                             list_add_tail(floor_pos, &elevator->list);
//                             elevator_weight += floor_passenger->weight;
//                             elevator->total_weight += floor_passenger->weight;
//                             elevator->total_cnt++;
//                         }
//                         else
//                         {
//                             break;
//                         }
//                     }

//                     mutex_unlock(&elevator_mutex);

//                     elevator_state = LOADING;
//                 }
//                 else
//                 {
//                     // If no passengers are waiting on the current floor, stay in UP or DOWN state
//                     // and continue moving to the next floor
//                 }
//             }

//             schedule_timeout(HZ);
//         }

//         schedule_timeout(HZ);
//     }

//     complete(&elevator_completion);
//     return 0;
// }

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
