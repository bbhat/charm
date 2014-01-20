///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_queue.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Queue Implementation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"

// A #define redefinition of OSW_QueueNode for ease of use
typedef _OS_QueueNode Node;  

// Function to initialize the queue 								 
void _OS_QueueInit(_OS_Queue * q)
{
	ASSERT(q);
	
	q->head = q->tail = NULL;
	q->count = 0;
}

// Function to insert an element into the queue. The key value determines the 
// location at which it will be inserted. This is a sorted queue on key value.
void _OS_QueueInsert(_OS_Queue * q, void * item, UINT64 key)
{
	Node *node, *new_node, *prev = 0;
	ASSERT(q && item);
	
	node = q->head;
	while(node)
	{
		if(node->key > key) break;
		prev = node;
		node = node->next;
	}

	new_node = (Node*) item;
	new_node->key = key;
	new_node->next = node;	
	if(!node)q->tail = new_node;	
	if(prev)
	{
	 	prev->next = new_node;
	}
	else
	{
		q->head = new_node;		
	}	  
	q->count++;
}

// Function to insert an element into the tail end of the queue.
void _OS_QueueInsertTail(_OS_Queue * q, void * item)
{
	Node *new_node;
	ASSERT(q && item);

	new_node = (Node*)item;
	new_node->next = NULL;

	if(q->tail)
	{
		(q->tail)->next = new_node;	
	}
	else
	{
		q->head = new_node; //first node
	}
	q->tail = new_node;
}

// Function to delete an item from the queue.
BOOL _OS_QueueDelete(_OS_Queue * q, void * item)
{
    Node * node, * prev = 0;
	Node * item_node = (Node*)item;
    ASSERT(q);
	
	node = q->head;
	while(node) 
	{
		if(node == item_node) break;
		prev = node;
		node = node->next;
	}
	
	if(node)   // Item exists
	{
		if(prev)	// Not the head node.
			prev->next = node->next;			
		else	// This is the head node
			q->head = node->next;	
		if(q->tail == node) // This is the last node
			q->tail = prev;

		q->count--;
		node->next = 0;
		
		return TRUE;
	}

	// Item not found in the queue	
	return FALSE;
}

// Function to get the first element from the Queue. 
void _OS_QueueGet(_OS_Queue * q, void ** item, UINT64 * key)
{
    Node * node;
    ASSERT(q);
	
	node = q->head;
	if(item) *item = (void *)node;
	if(node) 
	{
		q->head = node->next;		
		if(q->tail == node) q->tail = 0;
		if(key) *key = node->key;
		node->next = 0;
		q->count--;
	}
}

OS_Return _OS_QueuePeek(_OS_Queue * q, void ** item, UINT64 * key)
{
	ASSERT(q);
	
	if(item) *item = (void*) q->head;
	if(q->head)
	{
		if(key) *key = q->head->key;
		return SUCCESS;
	}
	return NO_DATA;
}
