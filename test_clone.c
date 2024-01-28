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

#define STACK_SIZE (4096)
#define INC_TIMES (1000000)
#define NTHREADS (2)

static volatile int s_sum = 0;

void add(void*, void*);

int main(void)
{
	int i;
	int pid;
	for (i = 0; i < NTHREADS; i++)
	{
		pid = thread_create(add, 0, 0);
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
		printf(1, "parent: process %d exited\n", pid);
	}
	exit();
}

/*
 * Increments shared variable `s_sum` INC_TIMES times
 * @param arg1 Ignored
 * @param arg2 Ignored
 * @revisions
 *   GJE p4b - Created
 */
void add(void* arg1, void* arg2)
{
	for (int i = 0; i < INC_TIMES; i++)
	{
		s_sum = s_sum + 1;
	}

	printf(1, "child: s_sum = %d\n", s_sum);
	exit();
}


