chARM 
=====
chARM is a Realtime Operating System developed from scratch that runs on ARM platform. chARM implements Earliest Deadline First (EDF) scheduling algorithm. Its periodic threading model allows creating tasks with arbitrary period, budget, deadline and phase having granularity and accuracy of few microseconds. chARM currently runs on TQ2440 development board that has Samsung's S3C2440 SOC. It is also currently being ported to a more powerful platform called Mini210s which has Samsung S5PV210 processor that runs upto 1 GHz. Each application in chARM is compiled independently and loaded from the ramdisk. Applications have their own address space and special care is taken to reduce switching b/w applications to be least expensive. Work is in progress to add MMU support.

Main Features of the RTOS
=========================

* From the beginning, one of the main goal was to keep the RTOS highly configurable. Most features of the OS can be enabled or disabled using compile time macros. This allows the developer to optimize the OS for smaller code size / lesser data / faster execution time / higher debug-ability or more features.

* The RTOS supports both periodic & aperiodic threads. This provides a great flexibility to write applications that make use of both periodic and priority based tasks.

* Periodic threads are scheduled using Earliest Deadline First (EDF) algorithm. The EDF algorithm is superior to Rate Monotonic Algorithm as it guarantees the schedulability under higher CPU utilization.

* Has built in support to accumulate run-time statistics about various Operating System parameters, CPU utilization etc.

* Keeps track of per task statistics such as accumulated run time, deadline miss count, Budget exceeded count etc.

* The RTOS makes use of a single hardware timer to control all EDF parameters (Period / Deadline / Budget / Phase). There is no single fixed timer interrupts in this RTOS. At each timer event, the RTOS computes when the next interrupt needs to be generated based on the current set of ready and waiting tasks.

* It configures the timer tick at a very high resolution (1.8963 uSec) to get good granularity. It smartly works around the resulting issue of smaller maximum interval while using 16 bit timers.

* Because the RTOS programs the main OS timer each time, the is a possibility of very slow drift. In order to eliminate this drift completely, it optionally makes use of an RTC timer timer which triggers every 10ms (configurable). At each resync interval, the main OS clock re-synchronizes itself to make the drift to be absolute zero.

* Multiple tasks with periods as much as 10,000 Hz are tested to be working fine with zero drift.

* The RTOS supports zero context store for periodic tasks which means for those periodic tasks which complete before its deadline / budget expiry it stores minimal context information. This is possible because each periodic task does not need to retain its registers between two periods.

* The RTOS is compiled separately from all applications. This helps to keep the address spaces separetely for enhanced security.

* Usually when each applications have their own address space, switching between tasks of different applications incur high cost which includes flushing instruction / data caches / TLBs etc. This time can run into few milliseconds. But chARM does this in a different way to provide least context switch time. Basically the total of 4GB address space is allocated to different applications & the kernel at the design time. Each application has non-overlapping address range allocated to it from the total 4GB address space. The page tables are setup in such a way that each application can access only its address range. This implementation means, we don't have to flush the caches or the TLBs. The total of 4GB is not a limiting factor in Embedded systems !

* The binaries corresponding to all applications, other data files are combined together to prepare a ramdisk file. This file is loaded when the RTOS boots up. This provides a read-only file system in chARM at run time.

* MMU support is being added. This enables paging and memory protection.

* Eventually I intend to add support for graphics using OpenGL

* Currently this RTOS runs on Mini210s (work in progress) & TQ2440.

* The git repository has all the necessary scripts to enable connecting and debugging using JLink & OpenOCD
