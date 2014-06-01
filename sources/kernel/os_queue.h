///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_queue.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_QUEUE_H
#define _OS_QUEUE_H

///////////////////////////////////////////////////////////////////////////////
// Hybrid Queue implementation which combines a Priority Queue and a NonPriority Queue
// This is a Hybrid queue where each element can participate in two queues at the same time.
// One priority queue and one non-priority queue. This routine is heavily used while managing
// task queues where there is a need to keep a single task in multiple queues
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Necessary Include Files
///////////////////////////////////////////////////////////////////////////////
#include "os_types.h"	// Include common data types being used

typedef struct _OS_HybridQNode
{	
	struct _OS_HybridQNode * np_next;	// NonPriority Queue Next
	struct _OS_HybridQNode * np_prev;	// NonPriority Queue Previous
	struct _OS_HybridQNode * p_next;	// Priority Queue Next
	struct _OS_HybridQNode * p_prev;	// Priority Queue Previous
	UINT64 key;							// Priority Key
	
} __attribute__ ((packed)) _OS_HybridQNode;

// Following type can be with for both Priority and NonPriority queues
typedef struct
{
	_OS_HybridQNode * head;
	_OS_HybridQNode * tail;
	UINT32 count;
	
} _OS_Queue;

///////////////////////////////////////////////////////////////////////////////
// Queue manipulation function
// Some of these functions intentionally don't return error values to keep them
// extremely efficient
///////////////////////////////////////////////////////////////////////////////

// Function to initialize the queue. 					 
void _OS_QueueInit(_OS_Queue * q);

// Function to insert an element into the queue. The key value determines the 
// location at which it will be inserted. This is a sorted queue on key value.
void _OS_PQueueInsertWithKey(_OS_Queue * q, _OS_HybridQNode * item, UINT64 key);
void _OS_NPQueueInsert(_OS_Queue * q, _OS_HybridQNode * item);

// Function to delete an item from the queue. 
// Returns true if the item is deleted, false otherwise
// This function does not actually validate if the element is in the queue, it is the 
// responsibility of the caller
BOOL _OS_PQueueDelete(_OS_Queue * q, _OS_HybridQNode * item);
BOOL _OS_NPQueueDelete(_OS_Queue * q, _OS_HybridQNode * item);

// Function to get the first element from the Queue. 
void _OS_PQueueGet(_OS_Queue * q, _OS_HybridQNode ** item);
void _OS_PQueueGetWithKey(_OS_Queue * q, _OS_HybridQNode ** item, UINT64 * key);
void _OS_NPQueueGet(_OS_Queue * q, _OS_HybridQNode ** item);

// Function to peek the first element from the Queue. 
BOOL _OS_QueuePeek(_OS_Queue * q, _OS_HybridQNode ** item);
BOOL _OS_QueuePeekWithKey(_OS_Queue * q, _OS_HybridQNode ** item, UINT64 * key);

#endif // _OS_QUEUE_H
