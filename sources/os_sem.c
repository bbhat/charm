///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_sem.c
//	Author: Bala B.
//	Description: OS Semaphore implementation
//
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_sem.h"

extern volatile void * g_current_task;
extern _OS_Queue g_ready_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_ap_wait_q;
extern void _OS_ReSchedule();

OS_Error OS_SemInit(OS_Sem *sem, INT16 pshared, UINT32 value)
{
	if(!sem)
		return ARGUMENT_ERROR;

	sem->count = value;
	_OS_QueueInit(&sem->periodic_task_queue);
	_OS_QueueInit(&sem->aperiodic_task_queue);
	return SUCCESS;
}

OS_Error OS_SemWait(OS_Sem *sem)
{
	OS_PeriodicTask * cur_task = (OS_PeriodicTask *) g_current_task;
	UINT32 intsts;
	
	if(!sem)
		return ARGUMENT_ERROR;

	while(1)
	{
		//disable interrupts
		OS_ENTER_CRITICAL(intsts);
			
		if(sem->count == 0)
		{
			//block the thread			
			if(IS_PERIODIC_TASK(cur_task->attributes))
			{
				_OS_QueueDelete(&g_ready_q, (void*)cur_task); //delete the current task from ready tasks queue
				_OS_QueueInsert(&sem->periodic_task_queue, (void*)cur_task, cur_task->alarm_time); //add the current task to the semaphore's blocked queue for periodic tasks
			}
			else
			{
				_OS_QueueDelete(&g_ap_ready_q, (void*)cur_task); //delete the current task from ready tasks queue
				_OS_QueueInsert(&sem->aperiodic_task_queue, (void*)cur_task, cur_task->alarm_time); //add the current task to the semaphore's blocked queue for aperiodic tasks
			}
			OS_EXIT_CRITICAL(intsts);
			_OS_ReSchedule();			
				
		}	
		else
		{
			sem->count--;	
			OS_EXIT_CRITICAL(intsts);
			break;
		}
		
	}	

	return SUCCESS;
}

OS_Error OS_SemPost(OS_Sem *sem)
{
	UINT32 intsts;
	OS_PeriodicTask* task = NULL;
	UINT64 key = 0;
	if(!sem)
		return ARGUMENT_ERROR;

	//disable interrupts
	OS_ENTER_CRITICAL(intsts);
	//sem->count++;
	if(sem->count == 0)
	{
		//unblock a task and remove from blocked queues		
		_OS_QueueGet(&sem->periodic_task_queue, (void**)&task, &key);//remove from the semaphore's blocked queue for periodic tasks
		if(task)//periodic task queue is not empty
		{
			_OS_QueueInsert(&g_ready_q,(void*)task,key);//place in the periodic task queue					
		}
		else//unblock an aperiodic job
		{
			_OS_QueueGet(&sem->aperiodic_task_queue, (void**)&task, &key);//remove from the semaphore's blocked queue for aperiodic tasks
			if(task)//aperiodic task queue is not empty
			{
				_OS_QueueInsert(&g_ap_ready_q,(void*)task,key);//place in the periodic task queue					
			}
		}
	}
	sem->count++;
	OS_EXIT_CRITICAL(intsts);
	if(task)
	{
		_OS_ReSchedule();
	}

	return SUCCESS;
}

OS_Error OS_SemDestroy(OS_Sem *sem)
{
	if(!sem)
		return ARGUMENT_ERROR;

	sem->count = 0;
	_OS_QueueInit(&sem->periodic_task_queue);//set pointers to NULL
	_OS_QueueInit(&sem->aperiodic_task_queue);//set pointers to NULL
	return SUCCESS;
}

OS_Error OS_SemGetvalue(OS_Sem *sem, INT32* val)
{
	if(!sem)
		return ARGUMENT_ERROR;
	if(val)
		*val = sem->count;	

	return SUCCESS;
}
