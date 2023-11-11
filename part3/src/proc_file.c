#include "elevator.h"
#include "proc_file.h"

static int read_p;

int elevator_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    read_p = 1;
    return 0;
}

ssize_t elevator_proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    // char buf[PROC_BUF_SIZE];
    char *buf = kmalloc(PROC_BUF_SIZE, GFP_KERNEL); // added
    int len = 0;

    if (!buf)
    {
        return -ENOMEM;
    }

    if (*ppos > 0 || count < PROC_BUF_SIZE)
    {
        kfree(buf);
        return 0;
    }

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

    struct list_head *pos;
    Passenger *p;
    len += sprintf(buf + len, "Elevator Status: ");
    list_for_each(pos, &elevator.list)
    {
        p = list_entry(pos, Passenger, list);
        len += sprintf(buf + len, "%s ", p->id);
    }

    len += sprintf(buf + len, "\n");
    len += sprintf(buf + len, "\n");

    for (int floor = 6; floor >= 1; floor--)
    {
        len += sprintf(buf + len, "[%c] Floor %d: %d",
                       (current_floor == floor) ? '*' : ' ', floor, floor_count[floor - 1]);

        list_for_each(pos, &floor_lists[floor - 1])
        {
            p = list_entry(pos, Passenger, list);
            len += sprintf(buf + len, " %s", p->id);
        }
        len += sprintf(buf + len, "\n");
    }

    len += sprintf(buf + len, "\nNumber of passengers: %d\n", elevator_count);
    len += sprintf(buf + len, "Number of passengers waiting: %d\n", FloorCountTotal());
    len += sprintf(buf + len, "Number of passengers serviced: %d\n", passengers_serviced);

    if (copy_to_user(ubuf, buf, len))
    {
        kfree(buf); // Free buffer in case of copy failure
        return -EFAULT;
    }
    *ppos = len;
    kfree(buf);
    return len;
}

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file)
{
    return 0;
}
