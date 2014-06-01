///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_queue.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Queue Implementation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_queue.h"
#include "os_core.h"

///////////////////////////////////////////////////////////////////////////////
//				Q Initialization
///////////////////////////////////////////////////////////////////////////////

// Function to initialize the Priority & NonPriority queues.
void _OS_QueueInit(_OS_Queue * q)
{
	ASSERT(q);
	q->head = q->tail = NULL;
	q->count = 0;
}

///////////////////////////////////////////////////////////////////////////////
//				Q Insertion
///////////////////////////////////////////////////////////////////////////////

// Function to insert an element into the queue. The key value determines the 
// location at which it will be inserted. This is a sorted queue on key value.
void _OS_PQueueInsertWithKey(_OS_Queue * q, _OS_HybridQNode * item, UINT64 key)
{
	_OS_HybridQNode *node, *prev;
	ASSERT(q && item);
	
	item->key = key;
	item->p_next = item->p_prev = NULL;

	if(!q->head || !q->tail) {
		q->head = q->tail = item;
	}
	else {
		node = q->head;
		while(node && (node->key <= key)) {
			node = node->p_next;
		}
		
		if(node) {
			item->p_next = node;
			prev = node->p_prev;
			node->p_prev = item;
			item->p_prev = prev;
			if(prev) prev->p_next = item;				
			else q->head = item;
		}
		else {
			q->tail->p_next = item;
			item->p_prev = q->tail;
			q->tail = item;	
		}
	}
	
	q->count++;
}

// Function to insert an element into the non-priority queue
// Inserts the new element at the tail
void _OS_NPQueueInsert(_OS_Queue * q, _OS_HybridQNode * item)
{
	ASSERT(q && item);

	item->np_next = NULL;

	if(q->tail) {
		(q->tail)->np_next = item;
	}
	else {
		q->head = item; //first node
	}
	item->np_prev = q->tail;
	q->tail = item;
	q->count++;
}

///////////////////////////////////////////////////////////////////////////////
//				Q Deletions
// Functions to delete an item from the queue. Returns true if the item is deleted.
// This function does not actually validate if the element is in the queue, it is the 
// responsibility of the caller
///////////////////////////////////////////////////////////////////////////////

BOOL _OS_PQueueDelete(_OS_Queue * q, _OS_HybridQNode * item)
{
    _OS_HybridQNode * next, * prev;
    ASSERT(q && item);

	next = item->p_next;
	prev = item->p_prev;
	item->p_next = NULL;
	item->p_prev = NULL;
	
	if(prev) {
		prev->p_next = next;
	}
	else {
		q->head = next;			// First element in the queue was deleted		
	}
	
	if(next) {
		next->p_prev = prev;
	}
	else {
		q->tail = prev;			// Last element in the queue was deleted
	}
	q->count--;
	return TRUE;
}

BOOL _OS_NPQueueDelete(_OS_Queue * q, _OS_HybridQNode * item)
{
    _OS_HybridQNode * next, * prev;
    ASSERT(q && item);

	next = item->np_next;
	prev = item->np_prev;
	item->np_next = NULL;
	item->np_prev = NULL;
	
	if(prev) {
		prev->np_next = next;
	}
	else {
		q->head = next;			// First element in the queue was deleted		
	}
	
	if(next) {
		next->np_prev = prev;
	}
	else {
		q->tail = prev;			// Last element in the queue was deleted
	}
	q->count--;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//				Q Get
// Functions to get the first element from the Queue. 
///////////////////////////////////////////////////////////////////////////////

void _OS_PQueueGet(_OS_Queue * q, _OS_HybridQNode ** item)
{
    _OS_HybridQNode * node;
    ASSERT(q);
	
	node = q->head;
	if(item) *item = node;
	if(node) 
	{
		q->head = node->p_next;
		if(!q->head) {
			q->tail = NULL;
		}
		else {
			q->head->p_prev = NULL;
		}
		node->p_next = node->p_prev = NULL;
		q->count--;
	}
}

void _OS_PQueueGetWithKey(_OS_Queue * q, _OS_HybridQNode ** item, UINT64 * key)
{
    _OS_HybridQNode * node;
    ASSERT(q && key);
	
	node = q->head;
	if(item) *item = node;
	if(node) 
	{
		*key = node->key;
		q->head = node->p_next;
		if(!q->head) {
			q->tail = NULL;
		}
		else {
			q->head->p_prev = NULL;
		}
		node->p_next = node->p_prev = NULL;
		q->count--;
	}
}

void _OS_NPQueueGet(_OS_Queue * q, _OS_HybridQNode ** item)
{
    _OS_HybridQNode * node;
    ASSERT(q);
	
	node = q->head;
	if(item) *item = (void *)node;
	if(node) 
	{
		q->head = node->np_next;
		if(!q->head) {
			q->tail = NULL;
		}
		else {
			q->head->np_prev = NULL;
		}
		node->np_next = node->np_prev = NULL;
		q->count--;
	}
}

///////////////////////////////////////////////////////////////////////////////
//				Q Peek
// Functions to get the first element from the Queue. 
///////////////////////////////////////////////////////////////////////////////

BOOL _OS_QueuePeek(_OS_Queue * q, _OS_HybridQNode ** item)
{
	ASSERT(q);
	
	if(item) *item = q->head;
	
	return (q->head) ? TRUE : FALSE;
}

BOOL _OS_QueuePeekWithKey(_OS_Queue * q, _OS_HybridQNode ** item, UINT64 * key)
{
	ASSERT(q && key);
	
	if(item) *item = q->head;
	if(q->head) {
		*key = q->head->key;
		return TRUE;
	}
	return FALSE;
}
