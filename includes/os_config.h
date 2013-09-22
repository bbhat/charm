///////////////////////////////////////////////////////////////////////////////
//    
//                        Copyright 2009-2013 xxxxxxx, xxxxxxx
//    File:    os_config.h
//    Author:    Bala B. (bhat.balasubramanya@gmail.com)
//    Description: OS Configuration options
//    
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_CONFIG_H
#define _OS_CONFIG_H

// OS Name
#define OS_NAME_STRING                    "chARM"

// Drivers to include
#define ENABLE_RTC                        1
#define ENABLE_RTC_ALARM                  1

// Instruction and Data Cache related
#define ENABLE_INSTRUCTION_CACHE          1
#define ENABLE_DATA_CACHE                 1

// L2 Cache preference. Not all platforms have L2 cache
// On S5PV210, enabling L2 cache also requires the L1 data cache to be enabled
// On S3C2440 there is no L2 cache
#if defined(SOC_S5PV210) && (ENABLE_DATA_CACHE == 1)
#define ENABLE_L2_CACHE                   1
#endif

// Process related
#define OS_PROCESS_NAME_SIZE              16

// Do we use Ramdisk? If we want processes to be separate entities, 
// then keeping them in the ramdisk is the only way
#define    ENABLE_RAMDISK                 1

// Task related configuration parameters
#define MIN_PRIORITY                      255
#define OS_IDLE_TASK_STACK_SIZE           0x40        // In Words
#define OS_STAT_TASK_STACK_SIZE           0x40        // In Words
#define STAT_TASK_PERIOD                  5000000     // 5 sec
#define OS_TASK_NAME_SIZE                 16
#define OS_MIN_USER_STACK_SIZE            256         // Minimum stack size in bytes

// This is the smallest period supported for periodic tasks.
// There will be an interrupt at every period. So setting this to
// a small period unnecessarily will result in performance impact.
// Ideally we can calculate this period dynamically by finding the task with smallest period.
// Further, all task periods & phase should be multiple of this period
// Deadlines & Budget are not dependent on this.
#define MIN_TASK_PERIOD                   1000       // in Microseconds.
#define MIN_TASK_BUDGET                   100        // 100 uSec

#define MAX_PROCESS_COUNT                 16          // This number is used to preallocate PCB
#define MAX_TASK_COUNT                    64          // This number is used to preallocate TCB
#define MAX_OPEN_FILES                    16          // This number is used to preallocate FILE strctures
#define MAX_SEMAPHORE_COUNT               64          // This number is used to preallocate Semaphore structures
#define MAX_MUTEX_COUNT                   64          // This number is used to preallocate Mutex structures

// Debug & Info related

// Define the Debug masks
typedef enum
{
    KLOG_CONTEXT_SWITCH       = (1 << 0),
    KLOG_OS_TIMER_ISR         = (1 << 1),
    KLOG_TBE_EXCEPTION        = (1 << 2),
    KLOG_DEADLINE_MISS        = (1 << 3),
    KLOG_PERIODIC_TIMER_ISR   = (1 << 4),
    KLOG_BUDGET_TIMER_ISR     = (1 << 5),
    KLOG_BUDGET_TIMER_SET     = (1 << 6),
    KLOG_WARNING              = (1 << 7),
    KLOG_GENERAL_INFO         = (1 << 8),
    KLOG_SEMAPHORE_DEBUG	  = (1 << 9),
	
    KLOG_MISC                 = (1 << 31)
    
} Klog_MaskType;

#define ENABLE_ASSERTS               0        // Enable ASSERT macros or not
#define OS_ENABLE_CPU_STATS          1        // Enable OS & CPU Stats
#define OS_WITH_VALIDATE_TASK        1

#define OS_KERNEL_LOGGING            0
#define OS_KLOG_MASK                 (KLOG_SEMAPHORE_DEBUG)
#define DEBUG_UART_CHANNEL           0

#endif // _OS_CONFIG_H
