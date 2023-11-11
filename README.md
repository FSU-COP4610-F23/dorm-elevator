<div style="margint: 200px;">
  
# Dorm Elevator
This project aims to provide a comprehensive understanding of system calls, kernel programming, concurrency, synchronization, and elevator scheduling algorithms. It consists of multiple parts that build upon each other to deepen your knowledge and skills in these areas.

In Part 1, you start by working with system calls. By adding system calls to a C/Rust program and verifying their correctness using the "strace" tool, you gain hands-on experience with system call integration and learn about the available system calls for your machine. This part lays the foundation for understanding how system calls interact with the kernel.

Part 2 takes you further into kernel programming. You will develop a kernel module called "my_timer" that retrieves and stores the current time using the "ktime_get_real_ts64()" function. This module creates a proc entry and allows you to read the current time and elapsed time since the last call. This part helps you understand how kernel modules work, how to interact with kernel functions, and how to use proc interfaces for communication.

Part 3 focuses on a more complex task: implementing a scheduling algorithm for a dorm elevator. You create a kernel module representing the elevator, supporting operations like starting, stopping, and issuing requests. The module also provides a "/proc/elevator" entry to display important elevator information. This part challenges you to manage concurrency, synchronization, and efficient scheduling within the kernel environment.

Each part of the project builds upon the knowledge and skills gained in the previous parts. Part 1 introduces you to system calls and their integration, which forms the basis for kernel programming in Part 2. Part 2 expands your understanding of kernel modules and communication through proc interfaces, setting the stage for the more advanced concepts explored in Part 3.

By completing this project, you acquire practical experience in system calls, kernel programming, concurrency, synchronization, and scheduling algorithms. These are essential skills for developing efficient and robust software systems, particularly in operating systems and low-level programming domains. Understanding system calls and kernel programming enables you to interact with and extend the functionality of the operating system, while concurrency, synchronization, and scheduling concepts are crucial for efficient resource management and multitasking in complex systems.


## Group Members
- **Jasmine Masopeh**: jdm21e@fsu.edu
- **Angela Fields**: js19@fsu.edu
- **JuanCarlos Alguera**: ab19@fsu.edu

## Division of Labor

### Part 1: System Call Tracing
  **Responsibilities**: 
    Create an empty C/Rust program named "empty".
    Make a copy of the "empty" program and name it "part1".
    Add exactly four system calls to the "part1" program. You can find the available system calls for your machine in "/usr/include/unistd.h".
    To verify that you have added the correct number of system calls, execute the following commands in the terminal:
  
    $   gcc -o empty empty.c
    $   strace -o empty.trace ./empty
    $   gcc -o part1 part1.c
    $   strace -o part1.trace ./part1

    To minimize the length of the output from strace, try to minimize the use of other function calls (e.g., stdlib.h) in your program.

    Note: Running strace on an empty C program will generate a number of system calls. Therefore, when using strace on your Part 1 code, it should produce four more system calls than the empty program.

- **Assigned to**: Jasmine Masopeh

### Part 2: Timer Kernel Module
 **Responsibilities**:
    In Unix-like operating systems, the time is often represented as the number of seconds since the Unix Epoch (January 1st, 1970). The task requires creating a kernel module named "my_timer" that utilizes the function ktime_get_real_ts64() to retrieve the time value, which includes seconds and nanoseconds since the Epoch.
    Develop a kernel module called my_timer that calls the ktime_get_real_ts64() function to obtain the current time. This module should store the time value.
    When the my_timer module is loaded using insmod, it should create a proc entry named "/proc/timer".
    When the my_timer module is unloaded using rmmod, the /proc/timer entry should be removed.
    On each read operation of "/proc/timer", utilize the proc interface to print both the current time and the elapsed time since the last call (if valid).

    To insert a kernel module:

    $ sudo insmod my_timer.ko
    To remove a kernel module:

    $ sudo rmmod my_timer.ko
    To check for your kernel module:

    $ lsmod | grep my_timer
    Example Usage:

    $ cat /proc/timer
    current time: 1518647111.760933999

    $ sleep 1

    $ cat /proc/timer
    current time: 1518647112.768429998
    elapsed time: 1.007495999

    $ sleep 3

    $ cat /proc/timer
    current time: 1518647115.774925999
    elapsed time: 3.006496001

    $ sleep 5

    $ cat /proc/timer
    current time: 1518647120.780421999
    elapsed time: 5.005496000


- **Assigned to**: Jasmine Masopeh

### Part 3a: Adding System Calls
  **Responsibilities**: 
    You should move the kernel to your /usr/src/ directory and create a soft link to it as so:

        $ sudo ln -s /usr/src/[kernel_version] ~/[kernel_version]
        $ cd ~/[kernel_version]

This will make it easier to modify from elsewhere instead of having to edit it in a restricted area.

Modify the kernel by adding three system calls to control the elevator and create passengers. Assign the following numbers to the system calls:

- 549 for issue_request()
- 548 for start_elevator()
- 550 for stop_elevator()

The respective function prototypes are as followed:

    int start_elevator(void)

The start_elevator() system call activates the elevator for service. From this point forward, the elevator exists and will begin to service requests. It returns 1 if the elevator is already active, 0 for a successful start, and -ERRORNUM if initialization fails or -ENOMEM if it couldn't allocate memory. The elevator is initialized with the following values:

    State: IDLE
    Current floor: 1
    Number of passengers: 0

    int issue_request(int start_floor, int destination_floor, int type)

The issue_request() system call creates a request for a passenger, specifying the start floor, destination floor, and type of passenger (0 for freshmen, 1 for sophomore, 2 for junior, 3 for senior). It returns 1 if the request is invalid (e.g., out of range or invalid type) and 0 otherwise.

    int stop_elevator(void)

The stop_elevator() system call deactivates the elevator. It stops processing new requests (passengers waiting on floors), but it must offload all current passengers before complete deactivation. Only when the elevator is empty can it be deactivated (state = OFFLINE). The system call returns 1 if the elevator is already in the process of deactivating and 0 otherwise.

You will need to make these files to add the system calls:

- [kernel_version]/syscalls/syscalls.c
- [kernel_version]/syscalls/Makefile
  
You will need to modify the following files to add te system calls:

- [kernel_version]/arch/x86/syscalls/syscall_64.tbl
- [kernel_version]/include/linux/syscalls.h
- [kernel_version]/Makefile

- **Assigned to**: Jasmine Masopeh, Juancarlos Alguera, Angela Fields

### Part 3b: Kernel Compilation
  **Responsibilities**:
need to disable certain certificates when adding system calls, follow the slides.

Compile the kernel with the new system calls. The kernel should be compiled with the following options:

    $ make menuconfig
    $ make -j$(nproc)
    $ sudo make modules_install
    $ sudo make install
    $ sudo reboot

Check that you installed your kernel by typing this into the terminal:

    $ uname -r

- **Assigned to**: Jasmine Masopeh, Juancarlos Alguera, Angela Fields

### Part 3c: Threads
- **Responsibilities**: 
Use a kthread to control the elevator movement.

- **Assigned to**: Angela Fields, Jasmine Masopeh

### Part 3d: Linked List
**Responsibilities**: 
Use linked lists to handle the number of passengers per floor/elevator.
- **Assigned to**: Juancarlos Alguera, Jasmine Masopeh

### Part 3e: Mutexes
- **Responsibilities**:
Use a mutex to control shared data access between floor and elevators.
- **Assigned to**: Jasmine Masopeh, Juancarlos Alguera, Angela Fields

### Part 3f: Scheduling Algorithm
- **Responsibilities**: 
The module must provide a proc entry named /proc/elevator. The following information should be printed (labeled appropriately):

The elevator's movement state:
OFFLINE: when the module is installed but the elevator isn't running (initial state)
IDLE: elevator is stopped on a floor because there are no more passengers to service
LOADING: elevator is stopped on a floor to load and unload passengers
UP: elevator is moving from a lower floor to a higher floor
DOWN: elevator is moving from a higher floor to a lower floor
The current floor the elevator is on
The elevator's current load (weight)
A list of passengers in the elevator
The total number of passengers waiting
The total number of passengers serviced
For each floor of the building, the following should be printed:

An indicator of whether or not the elevator is on the floor.
The count of waiting passengers.
For each waiting passenger, 2 characters indicating the passenger type and destination floor.
```
Example Proc File:

Elevator state: LOADING
Current floor: 4
Current load: 650 lbs
Elevator status: O5 F2 S4 S1

[ ] Floor 6: 1 S3
[ ] Floor 5: 0
[*] Floor 4: 2 F1 J2
[ ] Floor 3: 2 J4 J5
[ ] Floor 2: 0
[ ] Floor 1: 0

Number of passengers: 4
Number of passengers waiting: 5
Number of passengers serviced: 61

F is for freshmen, O is for sophomore, J is for junior, S is for seniors.

```

Interact with two provided user-space applications that enable communication with the kernel module:

- producer.c: creates passengers and issues requests to the elevator
    - $ ./producer [number_of_passengers]

- consumer.c: calls the start_elevator() or the stop_elevator() system call.
    - If the flag is --start, the program starts the elevator.
    - If the flag is --stop, the program stops the elevator.
You can use the following command to see your elevator in action:

    $ watch -n [snds] cat [proc_file]

The producer.c and consumer.c programs will be provided to you.


- **Assigned to**: Alex Brown, John Doe

## File Listing
```
elevator/
├── Makefile
├── part1/
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   ├── part1.trace
│   └── Makefile
├── part2/
│   ├── src/
│   └── Makefile
├── part3/
│   ├── src/
│   ├── tests/
│   ├── Makefile
│   └── sys_call.c
├── Makefile
└── README.md

```
# How to Compile & Execute

### Requirements
- **Compiler**: e.g., `gcc` for C/C++, `rustc` for Rust.
- **Dependencies**: List any libraries or frameworks necessary (rust only).

## Part 1

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
We don't really excute this part. The most you can do is look into the empty.trace and part1.trace to find the differences and how the syscalls in part1 affected it.

## Part 2

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in /part2
### Execution
To insert the kernel module:
```bash
make load 
```
or 
```bash
sudo insmod my_timer.ko 
```
For usage: 
```bash
cat /proc/timer
sleep 1
cat /proc/timer
sleep 3
cat /proc/timer
sleep 5
```
This will run the program ...


## Part 3

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in /part3
### Execution
```bash
make load
make watch
```
or 
```bash
sudo insmod my_timer.ko
watch -n1 cat /proc/elevator
```
This will run the program ...


## Bugs
- **Bug 1**: The elevator might miss some passengers but don't worry, it always goes back to them.


## Considerations
- We did our best to catch and fix elevator issues we noticed with the extra time from the super helpful extension. Jasmine also carried where she could when it was challenging to get help from lovely TAs due to people swarming/being busy, so not every one of our questions was answered. We also had other projects due for other classes, and between the three of us, we had six midterms from when the project was assigned till when it was expected. So please have mercy. 🙏
</div>
