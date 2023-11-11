#ifndef PROC_FILE_H
#define PROC_FILE_H

#include <linux/proc_fs.h>  // For proc file operations
#include <linux/seq_file.h> // If you use sequence file interfaces
#include <linux/uaccess.h>  // For copy_to/from_user

// Function prototypes for proc file operations
int elevator_proc_open(struct inode *sp_inode, struct file *sp_file); 
ssize_t elevator_proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos); 
int elevator_proc_release(struct inode *sp_inode, struct file *sp_file); 

#endif 