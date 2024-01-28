/*
 * Test routine for clone() system call
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
#define NTHREADS (4)
#define MODULO (1000)

static volatile int s_sum = 0;

void add(void*, void*);

int main(void)
{
	int i;
	int pid;
	
	lock_t lock;
	lock_init(&lock);

	for (i = 0; i < NTHREADS; i++)
	{
		pid = thread_create(add, (void*)&lock, 0);
		if (pid < 0)
		{
			printf(1, "thread_create failed!\n");
		}
		else
		{
			printf(1, "parent: process %d created\n", pid);
		}
	}
	for (i = 0; i < NTHREADS; i++)
	{
		pid = thread_join();
	}
	printf(1, "parent: final s_sum = %d\n", s_sum);
	exit();
}

/*
 * Increments shared variable `s_sum` INC_TIMES times
 * @param plock pointer to mutex lock
 * @param arg2 Ignored
 * @revisions
 *   GJE p4b - Created
 */
void add(void* plock, void* arg2)
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
		lock_release(plock);
	}

	exit();
}


