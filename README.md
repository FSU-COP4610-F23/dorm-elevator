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
- **Responsibilities**: [Please follow the instructions below to complete Part 1 of the task:

Create an empty C/Rust program named "empty".
Make a copy of the "empty" program and name it "part1".
Add exactly four system calls to the "part1" program. You can find the available system calls for your machine in "/usr/include/unistd.h".
To verify that you have added the correct number of system calls, execute the following commands in the terminal:
$ gcc -o empty empty.c
$ strace -o empty.trace ./empty
$ gcc -o part1 part1.c
$ strace -o part1.trace ./part1
To minimize the length of the output from strace, try to minimize the use of other function calls (e.g., stdlib.h) in your program.

Note: Running strace on an empty C program will generate a number of system calls. Therefore, when using strace on your Part 1 code, it should produce four more system calls than the empty program.

Please submit the following files:

empty.c/empty.rs
empty.trace
part1.c/part1.rs
part1.trace]
- **Assigned to**: Jasmine Masopeh

### Part 2: Timer Kernel Module
- **Responsibilities**: [Description]
- **Assigned to**: Jasmine Masopeh

### Part 3a: Adding System Calls
- **Responsibilities**: [Description]
- **Assigned to**: Alex Brown

### Part 3b: Kernel Compilation
- **Responsibilities**: [Description]
- **Assigned to**: Alex Brown, Jane Smith

### Part 3c: Threads
- **Responsibilities**: [Description]
- **Assigned to**: Alex Brown, Jane Smith

### Part 3d: Linked List
- **Responsibilities**: [Description]
- **Assigned to**: Jane Smith

### Part 3e: Mutexes
- **Responsibilities**: [Description]
- **Assigned to**: John Doe

### Part 3f: Scheduling Algorithm
- **Responsibilities**: [Description]
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
```bash
make run
```
This will run the program ...

## Part 2

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...


## Part 3

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...


## Bugs
- **Bug 1**: This is bug 1.
- **Bug 2**: This is bug 2.
- **Bug 3**: This is bug 3.

## Considerations
[Description]
# project-2-group-5
# project-2-group-5
