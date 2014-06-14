/**********************************************************************************
 *	
 *						Copyright 2014 xxxxxxx, xxxxxxx
 *	File:	Makefile
 *	Author:	Bala B. (bhat.balasubramanya@gmail.com)
 *	Description: Test program for os_queue
 *					This test is written to run on Mac
 *
 *********************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ASSERT(x) 	do { 																\
						if(!(x)) {														\
							printf("ASSERT Failed in " __FUNCTION__ ":" #x "\n");		\
							exit(0);													\
						}																\
					} while(0);

#define REQUIRE(x) 	do { 																\
						if(!(x)) {														\
							printf("ASSERT Failed in " __FUNCTION__ ":" #x "\n");		\
							exit(0);													\
						}																\
					} while(0);
					
#include "os_queue.c"		// Directly include the source file for os_queue

/**********************************************************************************
 * TODO: This test case makes use of random numbers. This makes it not predictable
 * This needs to be changed to use a fixed set of test values
 *********************************************************************************/

#define NUMBER_OF_NODES							20

BOOL alloc_nodes(_OS_Queue *npq, _OS_Queue *pq, UINT32 num_nodes);
void dealloc_nodes(_OS_Queue *npq, _OS_Queue *pq, UINT32 num_nodes);
void dealloc_nodes_2(_OS_Queue *npq, _OS_Queue *pq, UINT32 num_nodes);
void validate_pqueue(_OS_Queue *pq);

typedef struct Test_QNode
{	
	_OS_HybridQNode 	qp;
	UINT32				value;
	
} Test_QNode;

_OS_Queue npq;
_OS_Queue pq;

int main(void)
{
	_OS_QueueInit(&npq);
	_OS_QueueInit(&pq);
	
	srand(time(NULL));
	
	alloc_nodes(&npq, &pq, 100);
	validate_pqueue(&pq);
	dealloc_nodes(&npq, &pq, 100);
	validate_pqueue(&pq);
	alloc_nodes(&npq, &pq, 200);
	validate_pqueue(&pq);
	dealloc_nodes_2(&npq, &pq, 100);
	validate_pqueue(&pq);
	alloc_nodes(&npq, &pq, 200);
	validate_pqueue(&pq);
	dealloc_nodes(&npq, &pq, 100);
	validate_pqueue(&pq);
	dealloc_nodes_2(&npq, &pq, 300);
	validate_pqueue(&pq);
	
	return 0;
}

BOOL alloc_nodes(_OS_Queue *npq, _OS_Queue *pq, UINT32 num_nodes)
{
	int i;
	static UINT32 counter = 0;
	Test_QNode *node;
	BOOL ret = FALSE;
	
	ASSERT(npq);
	ASSERT(pq);
	
	printf("Allocating: ");	
	do {	
		// Allocate few Test_QNodes and link them using NP queue
		for(i = 0; i < num_nodes; i++) 
		{
			node = (Test_QNode *) malloc(sizeof(Test_QNode));
			if(!node) break;			
			node->value = counter++;
			_OS_NPQueueInsert(npq, (_OS_HybridQNode *)node);
			_OS_PQueueInsertWithKey(pq, (_OS_HybridQNode *)node, rand() % 100);
			
			printf(" %d", node->value);
		}
		
		ret = TRUE;
		
	} while(0);
	
	printf("\n");
	return ret;	
}

void dealloc_nodes(_OS_Queue *npq, _OS_Queue *pq, UINT32 num_nodes)
{
	Test_QNode *node;
	
	ASSERT(npq);
	ASSERT(pq);
	
	printf("Freeing:    ");
	
	while(num_nodes)
	{
		_OS_NPQueueGet(npq, (_OS_HybridQNode **)&node);
		if(!node) break;
		num_nodes--;
		
		printf(" %d", node->value);
		REQUIRE(_OS_PQueueDelete(pq, (_OS_HybridQNode *)node));
		free(node);			
	}
	
	printf("\n");
}

void dealloc_nodes_2(_OS_Queue *npq, _OS_Queue *pq, UINT32 num_nodes)
{
	Test_QNode *node;
	
	ASSERT(npq);
	ASSERT(pq);
	
	printf("Freeing:    ");
	
	while(num_nodes)
	{
		_OS_PQueueGet(pq, (_OS_HybridQNode **)&node);
		if(!node) break;
		num_nodes--;
		
		printf(" %d", node->value);
		REQUIRE(_OS_NPQueueDelete(npq, (_OS_HybridQNode *)node));
		free(node);			
	}
	
	printf("\n");
}

void validate_pqueue(_OS_Queue *pq)
{
	_OS_HybridQNode *node;
	UINT32 count = 0;
	UINT64 key;
	
	ASSERT(pq);	
	
	node = pq->head;
	if(node)
	{
		key = node->key;
		do
		{
			count++;
			REQUIRE(key <= node->key);
			key = node->key;
			node = node->p_next;
		} 
		while(node);
	}
	
	// Validate the count
	REQUIRE(count == pq->count);
	
	printf("Validated PQ (count %d)\n", count);
}
