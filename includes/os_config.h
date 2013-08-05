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

// Since the OS timer is not a fixed interval timer, there can be a very small drift
// over a long period of time. The RTOS actually calculates the time spent in the 
// timer ISR and deducts it from the next task budget. However, there can be a drift 
// worth few of instructions b/w the time we read the timer and adjust and write it back.
// Using another fixed interval timer, we can resynchronize periodically.
#define ENABLE_SYNC_TIMER                 1
// IMPORTANT NOTE: The below interval should be <= MAX_TIMER1_INTERVAL_uS
// An optimal value for this depends on the number of interrupts expected per
// second based on the task intervals. The ideal value for this would be
// one SYNC for every 100 OS timer interrupts.
#define SYNC_TIMER_INTERVAL               10000        // in Microseconds.

// The OS Timer is not a fixed interval timer. Each time we set the timer, we adjust for the
// time spent in the ISR. We do this by reading the timer value and reducing that time 
// from the new timeout desired. However some platforms don't support updating the timeout
// without affecting the timer operation. This means we cannot adjust for this time.
// The following constant adds a fixed delay to the calculated elapsed time while setting the
// OS timer. This is basically to account for the time spent b/w reading OS timer value
// to setting it.
// If you are seeing "SYNC interrupt not expected" errors, it can be eliminated by
// suitably selecting SYNC_TIMER_INTERVAL and OS_TIMER_ELAPSED_TIME_ADJ
#ifdef DEBUG
#define OS_TIMER_ELAPSED_TIME_ADJ         5            // In terms of OS Timer tick count
#else
#define OS_TIMER_ELAPSED_TIME_ADJ         3            // In terms of OS Timer tick count
#endif

// Task related configuration parameters
#define MIN_PRIORITY                      255
#define OS_IDLE_TASK_STACK_SIZE           0x40        // In Words
#define OS_STAT_TASK_STACK_SIZE           0x40        // In Words
#define STAT_TASK_PERIOD                  5000000     // 5 sec
#define OS_TASK_NAME_SIZE                 16
#define OS_MIN_USER_STACK_SIZE            256         // Minimum stack size in bytes

// Time for the first scheduling interrupt to begin. This should be <= MAX_TIMER0_INTERVAL_uS
// Just few micro seconds are enough. This is to adjust the time gap b/w the timer setup
// and the OS to be ready to handle interrupts. If this delay is not enough, it will eat from
// the budget of the first job. Few microseconds are enough actually.
#define OS_FIRST_SCHED_DELAY              10000       // 10 ms

// Following values depend a lot on the timer resolution which is not very good in this case
#define TASK_MIN_PERIOD                   100         // 100 uSec
#define TASK_MIN_BUDGET                   100         // 100 uSec

#define MAX_PROCESS_COUNT                 16          // This number is used to preallocate PCB
#define MAX_TASK_COUNT                    64          // This number is used to preallocate TCB
#define MAX_OPEN_FILES                    16          // This number is used to preallocate FILE strctures

// Debug & Info related

// Define the Debug masks
typedef enum
{
    KLOG_CONTEXT_SWITCH       = (1 << 0),
    KLOG_OS_TIMER_ISR         = (1 << 1),
    KLOG_TBE_EXCEPTION        = (1 << 2),
    KLOG_OS_TIMER_SET         = (1 << 3),
    KLOG_SYNC_TIMER_ISR       = (1 << 4),
    KLOG_WARNING              = (1 << 5),
    KLOG_GENERAL_INFO         = (1 << 6),
    
    KLOG_MISC                 = (1 << 31)
    
} Klog_MaskType;

#define OS_ENABLE_CPU_STATS          1        // Enable OS & CPU Stats
#define OS_WITH_VALIDATE_TASK        1

#define OS_KERNEL_LOGGING            0
#define OS_KLOG_MASK                 (KLOG_CONTEXT_SWITCH)
#define DEBUG_UART_CHANNEL           0

#endif // _OS_CONFIG_H
