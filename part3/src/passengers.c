
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
