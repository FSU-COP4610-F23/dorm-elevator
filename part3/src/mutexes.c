size_t mutex_demo_read(struct file *filep, char __user *buf, size_t len, loff_t *offp) {
    char local_buffer[256];
    int bytes = 0;
    int i;

    mutex_lock(&floor_mutex);
    bytes += snprintf(local_buffer + bytes, sizeof(local_buffer) - bytes, "Current elevator position: %d\n", elevator_position);

    bytes += snprintf(local_buffer + bytes, sizeof(local_buffer) - bytes, "People waiting on each floor: ");
    for(i = 0; i < MAX_FLOORS; i++) {
        bytes += snprintf(local_buffer + bytes, sizeof(local_buffer) - bytes, "Floor %d: %d ", i, current_floor[i]);
    }
    bytes += snprintf(local_buffer + bytes, sizeof(local_buffer) - bytes, "\n");
    mutex_unlock(&floor_mutex);

    return simple_read_from_buffer(buf, len, offp, local_buffer, bytes);
}