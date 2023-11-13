#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jasmine Masopeh");
MODULE_DESCRIPTION("Kernel Module with Timer");
MODULE_VERSION("1.0");

static struct timespec64 last_time; // Store the last recorded time

static int my_timer_read(struct seq_file *m, void *v)
{
    struct timespec64 now;
    ktime_get_real_ts64(&now); // Get the current time

    // Print the current time to the seq_file
    seq_printf(m, "Current Time: %lld.%09ld\n", now.tv_sec, now.tv_nsec);

    // Check if it's not the first read
    if (last_time.tv_sec != 0 || last_time.tv_nsec != 0)
    {
        struct timespec64 elapsed;
        elapsed.tv_sec = now.tv_sec - last_time.tv_sec;
        elapsed.tv_nsec = now.tv_nsec - last_time.tv_nsec;

        if (elapsed.tv_nsec < 0)
        {
            elapsed.tv_sec--;
            elapsed.tv_nsec += 1000000000;
        }

        // Print the elapsed time to the seq_file
        seq_printf(m, "Elapsed Time: %lld.%09ld\n", elapsed.tv_sec, elapsed.tv_nsec);
    }

    last_time = now; // Update the last recorded time

    return 0;
}

static int my_timer_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_timer_read, NULL);
}

static const struct proc_ops my_timer_fops = {
    .proc_open = my_timer_open,     // Opening the file
    .proc_read = seq_read,          // Reading data
    .proc_lseek = seq_lseek,        // Seeking in the file
    .proc_release = single_release, // Releasing resources
};

static int __init my_timer_init(void)
{
    struct proc_dir_entry *entry;

    entry = proc_create("timer", 0444, NULL, &my_timer_fops);
    if (!entry)
    {
        pr_err("Failed to create /proc/timer entry\n");
        return -ENOMEM;
    }

    // Initialize last_time as a timespec64 structure with zero values
    last_time.tv_sec = 0;
    last_time.tv_nsec = 0;

    pr_info("my_timer module initialized\n"); // Log module
    return 0;
}

static void __exit my_timer_exit(void)
{
    // Remove the /proc/timer entry
    remove_proc_entry("timer", NULL);
    pr_info("my_timer module removed\n"); // Log module removal
}

module_init(my_timer_init);
module_exit(my_timer_exit);
