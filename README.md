# Kernel-InfoTracker-Process-and-Thread-Insights
Designed a Character device driver and integrated sysfs for efficient access to process attributes such as ID, priority, command name, parent ID, context switches, as well as thread count, open files, and max stack usage.


Key Requirements
Character Device: /dev/cs614_device
Sysfs Directory: /sys/kernel/cs614_sysfs
Sysfs File: /sys/kernel/cs614_sysfs/cs614_value

## Part 1: Single Process Access
Objective: Implement a character driver that supports the following functionalities for a single process.

Supported Commands:
Pid (command = 0): Return the process ID of the user process.
Static Priority (command = 1): Return the static priority of the user process.
Command Name (command = 2): Return the command name of the user process.
Ppid (command = 3): Return the real parent's process ID of the user process.
Number of Voluntary Context Switches (command = 4): Return the number of voluntary context switches of the user process.
Implementation Details:
Write operation on /sys/kernel/cs614_sysfs/cs614_value sets the command.
Read operation on /dev/cs614_device fetches the result based on the command.
Commands beyond '7' return -EINVAL.
The module assumes a single process uses the driver at any given time.

## Part 2: Multiprocess Access
Objective: Extend the driver to support multiple processes simultaneously, ensuring correct results even with interleaved operations.

Key Features:
Handles simultaneous writes and reads from multiple processes.
Ensures that each process receives the correct result for its specific command.

## Part 3: Multithreaded Access
Objective: Extend the driver to support multithreaded access with additional functionalities.

Supported Commands:
Number of Threads (command = 5): Return the number of threads created by the user process.
Number of Open Files (command = 6): Return the number of files opened by the process/thread.
Max Stack Usage PID (command = 7): Return the PID of the thread with the maximum user stack usage among all threads including the main process.
