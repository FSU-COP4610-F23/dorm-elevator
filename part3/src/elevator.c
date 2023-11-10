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
    int type_weights[NUM_PASSENGER_TYPES] = {100, 150, 200, 250};
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

// Function to load passengers onto the elevator
void load_passengers(int current_floor)
{
    // mutex_lock(&elevator_mutex);
    struct list_head *floor_pos, *floor_temp;
    Passenger *floor_passenger;
    list_for_each_safe(floor_pos, floor_temp, &floor_lists[current_floor - 1])
    {
        floor_passenger = list_entry(floor_pos, Passenger, list);
        if (elevator_weight + floor_passenger->weight <= MAX_LOAD && elevator_count + 1 <= 5)
        {
            list_del(floor_pos);
            list_add_tail(floor_pos, &elevator.list);
            elevator_weight += floor_passenger->weight;
            elevator_count++;
            floor_count[current_floor - 1]--;
        }
    }
    // mutex_unlock(&elevator_mutex);
}

void unload_passengers(void)
{
    // mutex_lock(&elevator_mutex);
    struct list_head *pos, *temp;
    Passenger *p;
    list_for_each_safe(pos, temp, &elevator.list)
    {
        p = list_entry(pos, Passenger, list);
        if (p->destination == current_floor)
        {
            list_del(pos);
            kfree(p);
            elevator_weight -= p->weight;
            elevator_count--;
            passengers_serviced++;
        }
    }
    // mutex_unlock(&elevator_mutex);
}

void searchNextEmpty(void)
{
    // mutex_lock(&elevator_mutex);
    bool emptyFloor[6] = {true, true, true, true, true, true};
    bool empty = true;
    for (int i = 0; i < 6; i++)
    {
        // check for any passengers on every floor
        if (!list_empty(&floor_lists[i]))
        {
            emptyFloor[i] = false; // sets false for specific floor
            empty = false;         // false if at least 1 passenger exists on any floor
        }
    }
    if (empty == true)
    {
        elevator_state = IDLE; // state for IDLE
        // mutex_unlock(&elevator_mutex);
        return;
    }
    if (!emptyFloor[current_floor - 1])
    {
        elevator_state = LOADING;
        // mutex_unlock(&elevator_mutex);
        return;
    }
    for (int i = 1; i < 6; i++)
    {
        int add, sub;
        add = current_floor + i - 1;
        sub = current_floor - i - 1;
        if (add <= 5 && !emptyFloor[add])
        {
            elevator_state = UP;
            elevator_dest = current_floor + i;
            // mutex_unlock(&elevator_mutex);
            return;
        }
        else if (sub >= 0 && !emptyFloor[sub])
        {
            elevator_state = DOWN;
            elevator_dest = current_floor - i;
            // mutex_unlock(&elevator_mutex);
            return;
        }
    }
}

// Elevator thread function
static int elevator_thread_function(void *data)
{
    while (!kthread_should_stop())
    {
        mutex_lock(&elevator_mutex);
        if (elevator_state == IDLE)
        { // idle when elevator is empty
            // mutex_unlock(&elevator_mutex);
            searchNextEmpty(); // if floors all empty, stay in IDLE state
                               // mutex_lock(&elevator_mutex);
        }
        else if (elevator_state == UP || elevator_state == DOWN)
        {
            while (current_floor != elevator_dest)
            {
                if (elevator_state == UP)
                {
                    // mutex_unlock(&elevator_mutex);
                    msleep(2000);
                    // mutex_lock(&elevator_mutex);
                    current_floor = current_floor + 1;
                }
                else if (elevator_state == DOWN)
                {
                    // mutex_unlock(&elevator_mutex);
                    msleep(2000);
                    // mutex_lock(&elevator_mutex);
                    current_floor = current_floor - 1;
                }
            }
            elevator_state = LOADING;
        }
        else if (elevator_state == LOADING)
        {
            if (!list_empty(&elevator.list))
            {
                // mutex_unlock(&elevator_mutex);
                msleep(1000);
                // mutex_unlock(&elevator_mutex);
                unload_passengers();
                // mutex_lock(&elevator_mutex);
            }
            if (!list_empty(&floor_lists[current_floor - 1]))
            {
                // mutex_unlock(&elevator_mutex);
                msleep(1000);
                // mutex_unlock(&elevator_mutex);
                load_passengers(current_floor);
                // mutex_lock(&elevator_mutex);
            }
            if (list_empty(&elevator.list))
            {
                // mutex_unlock(&elevator_mutex);
                searchNextEmpty();
                // mutex_lock(&elevator_mutex);
            }
            else
            {
                Passenger *p;
                p = list_first_entry(&elevator.list, Passenger, list);
                elevator_dest = p->destination;
                if (elevator_dest > current_floor)
                    elevator_state = UP;
                else if (elevator_dest < current_floor)
                    elevator_state = DOWN;
                else
                {
                    elevator_state = LOADING;
                }
            }
        }
        mutex_unlock(&elevator_mutex);
    }

    return exit_elevator();
}

int exit_elevator(void)
{
    mutex_lock(&elevator_mutex);
    while (!list_empty(&elevator.list))
    {
        Passenger *p;
        p = list_first_entry(&elevator.list, Passenger, list);
        elevator_dest = p->destination;
        if (elevator_dest > current_floor)
            elevator_state = UP;
        else if (elevator_dest < current_floor)
            elevator_state = DOWN;

        while (current_floor != elevator_dest)
        {
            if (elevator_state == UP)
            {
                // mutex_unlock(&elevator_mutex);
                msleep(2000);
                // mutex_lock(&elevator_mutex);
                current_floor = current_floor + 1;
            }
            else if (elevator_state == DOWN)
            {
                // mutex_unlock(&elevator_mutex);
                msleep(2000);
                // mutex_lock(&elevator_mutex);
                current_floor = current_floor - 1;
            }
        }

        elevator_state = LOADING;
        msleep(1000);
        unload_passengers();
    }
    mutex_unlock(&elevator_mutex);
    // exit while loop means elevator is empty
    elevator_state = OFFLINE;
    return 1;
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

int stop_elevator(void)
{
    kthread_stop(elevator_thread);
    // Lock the elevator_mutex to ensure exclusive access
    mutex_lock(&elevator_mutex);
    if (elevator_state == OFFLINE)
    { // If the elevator is empty and all passengers have been serviced
        mutex_unlock(&elevator_mutex);
        return 0; // Return 0 for successful deactivation
    }
    mutex_unlock(&elevator_mutex); // Unlock the elevator_mutex
    return 1;                      // Return 1 if the elevator is not empty and deactivation is in progress
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
