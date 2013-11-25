///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_queue.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
//	Note: This file implements a singly linked list. Singly linked lists are
// 	not efficient when we have to delete some items from the middle. We do
//  have this requirement in the case of user calling 'Sleep' method. However
//	just for the sake of 'Sleep' we are not going in for a doubly linked list
//	as 'Sleep' will be very rarely used. On the other hand maintaining a
//	doubly linked list will have more overhead for normal scheduling operations.
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_QUEUE_H
#define _OS_QUEUE_H

///////////////////////////////////////////////////////////////////////////////
// Necessary Include Files
///////////////////////////////////////////////////////////////////////////////
#include "os_types.h"	// Include common data types being used
#include "os_core.h"  // For OS_Error definition

// Typedef for Queue node
typedef struct _OS_QueueNode _OS_QueueNode;
struct _OS_QueueNode
{	
	_OS_QueueNode * next;   
	UINT64 key;				
};

// Public typedefs
typedef struct
{
	_OS_QueueNode * head;
	_OS_QueueNode * tail;
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
void _OS_QueueInsert(_OS_Queue * q, void * item, UINT64 key);
void _OS_QueueInsertTail(_OS_Queue * q, void * item);

// Function to delete an item from the queue.
void _OS_QueueDelete(_OS_Queue * q, void * item);

// Function to get the first element from the Queue. 
void _OS_QueueGet(_OS_Queue * q, void ** item, UINT64 * key);

// Function to peek the first element from the Queue. 
OS_Error _OS_QueuePeek(_OS_Queue * q, void ** item, UINT64 * key);

#endif // _OS_QUEUE_H
