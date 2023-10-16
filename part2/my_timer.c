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

static ktime_t my_current_time; // Store the current time
static ktime_t last_time;       // Store the last recorded time

// Read function for the /proc/timer entry
static int my_timer_read(struct seq_file *m, void *v) {
    ktime_t now = ktime_get_real(); // Get the current time
    struct timespec64 ts_now = ktime_to_timespec64(now); // Convert current time to timespec

    // Print the current time to the seq_file
    seq_printf(m, "Current Time: %lld.%09ld\n", ts_now.tv_sec, ts_now.tv_nsec);

    // Check if it's not the first read
    if (ktime_compare(last_time, ktime_set(0, 0)) != 0) {
        ktime_t elapsed = ktime_sub(now, last_time); // Calculate elapsed time
        struct timespec64 ts_elapsed = ktime_to_timespec64(elapsed); // Convert elapsed time to timespec

        // Print the elapsed time to the seq_file
        seq_printf(m, "Elapsed Time: %lld.%09ld\n", ts_elapsed.tv_sec, ts_elapsed.tv_nsec);
    }

    last_time = now; // Update the last recorded time

    return 0;
}

// Open function for the /proc/timer entry
static int my_timer_open(struct inode *inode, struct file *file) {
    return single_open(file, my_timer_read, NULL);
}

// Define file operations for the /proc/timer entry
static const struct proc_ops my_timer_fops = {
    .proc_open = my_timer_open,   // Opening the file
    .proc_read = seq_read,       // Reading data
    .proc_lseek = seq_lseek,     // Seeking in the file
    .proc_release = single_release, // Releasing resources
};

// Initialization function for the kernel module
static int __init my_timer_init(void) {
    struct proc_dir_entry *entry;

    entry = proc_create("timer", 0444, NULL, &my_timer_fops); // Create /proc/timer entry
    if (!entry) {
        pr_err("Failed to create /proc/timer entry\n"); // Handle entry creation failure
        return -ENOMEM;
    }

    my_current_time = ktime_get_real(); // Get the initial current time
    last_time = ktime_set(0, 0); // Initialize last_time

    pr_info("my_timer module initialized\n"); // Log module initialization
    return 0;
}

// Cleanup function for the kernel module
static void __exit my_timer_exit(void) {
    remove_proc_entry("timer", NULL); // Remove the /proc/timer entry
    pr_info("my_timer module removed\n"); // Log module removal
}

// Define the module initialization and cleanup functions
module_init(my_timer_init);
module_exit(my_timer_exit);


