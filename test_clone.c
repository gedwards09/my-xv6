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

volatile int s_sum = 0;

int thread_create(void (*start_routine)(void*, void*), void* arg1, void* arg2);
void add(void*, void*);

int main(void)
{
	int pid = thread_create(add, 0, 0);
	if (pid < 0)
	{
		printf(1, "thread_create failed!\n");
	}
	else
	{
		for (int i=0; i < 100; i++)
		{
			yield();
		}
		printf(1, "parent: process %d created\n", pid);
	}
	exit();
}

/*
 * Creates new thread executing start_routine.
 * Stack must be freed by parent process on exit
 * @param start_routine pointer to the routine to run
 * @param arg1 first argument passed to start_routine
 * @param arg2 second argument passed to start_routine
 * @returns pid of created process if successful, -1 otherwise
 * @revisions
 *   GJE p4b - Created
 */
int thread_create(void (*start_routine)(void*, void*), void* arg1, void* arg2)
{
	char* stack = malloc(STACK_SIZE);
	return clone(start_routine, arg1, arg2, stack);
}

/*
 * Increments shared variable `sum` 1,000,000 times
 * @param arg1 Ignored
 * @param arg2 Ignored
 * @revisions
 *   GJE p4b - Created
 */
void add(void* arg1, void* arg2)
{
	for (int i = 0; i < INC_TIMES; i++)
	{
		s_sum += 1;
	}

	printf(1, "child: s_sum = %d", s_sum);
	exit();
}


