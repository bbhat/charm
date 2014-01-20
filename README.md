chARM 
=====
chARM is a Realtime Operating System developed from scratch that runs on ARM platform. chARM implements Earliest Deadline First (EDF) scheduling algorithm. Its periodic threading model allows creating tasks with arbitrary period, budget, deadline and phase having granularity and accuracy of few microseconds. chARM currently runs on Mini210s development board which has Samsung S5PV210 SOC with Cortex-A8 core operating at 1GHz. It also runs on TQ2440 development board which has Samsung S3C2440 SOC. Each application in chARM is compiled and built independently from the kernel. A ramdisk File System image is prepared with all applications, data and loaded by the kernel when it starts. The main goal of this OS is to be able to use it in safety critical hard realtime applications. This repository is self contained and has all the tools and scripts necessary to build, run this OS and also debug using GDB. Each application and the kernel has its own address space and are protected from each other.

Main Features of the RTOS
=========================

* From the beginning, one of the main goal was to keep the RTOS highly configurable. Most features of the OS can be enabled or disabled using compile time macros. This allows the us to optimize the OS for smaller code / memory / faster execution time / higher debug-ability or more features.

* The RTOS supports both periodic & aperiodic threads. This provides a great flexibility to write applications that make use of both periodic and priority based tasks.

* Periodic threads are scheduled using Earliest Deadline First (EDF) algorithm. The EDF algorithm is superior to Rate Monotonic Algorithm as it guarantees the schedulability under higher CPU utilization. The order in which different tasks having the same period run can also be easily controlled. So by intelligently creating tasks with right values, we can achive synchronization without using locks.

* Each application in chARM is compiled and built independently from the kernel. 

* A ramdisk File System image is prepared with all applications, data and loaded by the kernel when it starts. This provides a read-only file system in chARM at run time.

* The OS accumulates run-time statistics about various Operating System parameters such as CPU / Memory utilization etc. This can be disabled using configuration parameters.

* There is a 'System Monitor' application which can be used to display oervall OS statistics and per task statistics such as accumulated run time, deadline miss count, Budget exceeded count etc.

* The RTOS makes use of two timers one of which is a fixed interval periodic timer, another is a variable interval timer which is used to track the budget / deadline. The fixed interval timer used for tracking period / phase gurantees that there are no time drifts.

* The RTOS supports zero context store for periodic tasks which means for those periodic tasks which complete before its deadline / budget expiry it stores minimal context information. This is possible because each periodic task does not need to retain its registers between two periods.

* Usually when each applications have their own address space, switching between tasks of different applications incur high cost which includes flushing instruction / data caches / TLBs etc. This time can run into few milliseconds. But chARM does this in a different way to provide least context switch time. Basically the total of 4GB address space is allocated to different applications & the kernel at the design time. Each application has non-overlapping address range allocated to it from the total 4GB address space. The page tables are setup in such a way that each application can access only its address range. This implementation means, we don't have to flush the caches or the TLBs. The total of 4GB is not a limiting factor in Embedded systems !

* Full MMU support and memory protection is available. The Kernel process can be configured to use page sizes of 1MB / 64KB / 4KB. User processes can be configured to use page sizes of 64K / 4K

* A comprehensive and flexible device driver framework is provided. This framework makes it easy to write new kernel drivers that can be easily and accessed from other kernel drivers or user applications. Many common tasks such as queuing IOs, blocking, mechanisms for calling from user space etc are automatically handled by the driver framework.

* Semaphore implementation is specifically adapted for the periodic EDF scheduler. If there are multiple tasks waiting for a semaphore, periodic tasks get the first preference when the semaphore becomes available followed by aperiodic tasks. Within the periodic tasks, a task with earliest approaching deadline is given priority (not necessarily mean a task with the smallest period). This behavior ideal for use with an EDF scheduler. The periodic tasks are given preference in the order of their priority.

* Currently this RTOS runs on Mini210s & TQ2440 development boards. The timing and accuracy of the scheduled has been excellent.

* The git repository has all the necessary tools & scripts to enable connecting and debugging using JLink & OpenOCD


### Mini210s Development Board

[Mini210s | S5PV210 ARM Cortex-A8 Board](http://www.friendlyarm.net/products/mini210s)

### Mini2440 / TQ2440 Development Board

[Mini2440 | S3C2440 ARM9 Board](http://www.friendlyarm.net/products/mini2440)
