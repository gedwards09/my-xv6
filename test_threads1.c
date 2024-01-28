/*
 * Test routine for thread and lock libraries.
 * Test memory management between threads sharing address space
 *
 * @author Greg Edwards
 * @version 1.0
 * @revisions
 *   GJE p4b - Created
 */

#include "types.h"
#include "user.h"
#include "rand.h"

#define INC_TIMES (1000)
#define PGSIZE (4096)
#define NTHREADS (4)
#define MODULO (1000)


typedef struct __node_t node_t;
struct __node_t
{
	int data;
	node_t* next;
};

/* head of linked list */
static node_t* s_pHead = 0;
/* shared summation variable */
static volatile int s_sum = 0;


void doAdd(void*, void*);
void add(int data);
node_t* makeNode(int data);
void freeNodes(void);

int main(void)
{
	int i;
	int pid;
	char* p;
	
	lock_t lock;
	lock_init(&lock);

	/* allocate large chunck of memory */
	p = malloc(PGSIZE);

	for (i = 0; i < NTHREADS; i++)
	{
		pid = thread_create(doAdd, (void*)&lock, 0);
		if (pid < 0)
		{
			printf(1, "thread_create failed!\n");
		}
		else
		{
			printf(1, "parent: process %d created\n", pid);
		}
	}

	/* release memory before waiting for threads */
	free(p);

	for (i = 0; i < NTHREADS; i++)
	{
		pid = thread_join();
	}
	printf(1, "parent: final s_sum = %d\n", s_sum);

	freeNodes();		

	exit();
}

/*
 * Increments shared variable `s_sum` INC_TIMES times
 * and adds current value to linked list
 * @param plock pointer to mutex lock
 * @param arg2 Ignored
 * @revisions
 *   GJE p4b - Created
 */
void doAdd(void* plock, void* arg2)
{
	int curval;

	for (int i = 0; i < INC_TIMES; i++)
	{
		lock_acquire(plock);
		curval = s_sum;
		if (rand0() % MODULO == 0)
		{
			yield();
		}
		s_sum = curval + 1;
		add(s_sum);
		lock_release(plock);
	}


	exit();
}

/*
 * Insert data at front of linked list
 */
void add(int data)
{
	node_t* pNode;

	pNode = makeNode(data);
	if (s_pHead == 0)
	{
		s_pHead = pNode;
	}
	else
	{
		pNode->next = s_pHead;
		s_pHead = pNode;
	}
}

/*
 * Create new node with the specified data
 */
node_t* makeNode(int data)
{
	node_t* pNode;
	pNode = malloc(sizeof(node_t));
	pNode->data = data;
	pNode->next = 0;
	return pNode;
}

/*
 * Destroy all nodes from the linked list
 */
void freeNodes(void)
{
	node_t* pCurrent;
	node_t* pTmp;
	pCurrent = s_pHead;
	while (pCurrent != 0)
	{
		pTmp = pCurrent->next;
		free(pCurrent);
		pCurrent = pTmp;
	}
}
