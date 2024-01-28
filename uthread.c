/*
 * Library functions for managing process threads
 * @author Greg Edwards
 * @version 1.0
 * @revisions
 *   GJE p4b - Created
 */

#include "types.h"
#include "user.h"
#include "uthread.h"

#define PGSIZE (4096)

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
	char* stack = malloc(PGSIZE);
	return clone(start_routine, arg1, arg2, stack);
}

/*
 * Waits for thread sharing the address space to exit.
 * @returns pid of the exited thread if successful, -1 otherwise
 * @revisions
 *   GJE p4b - Created
 */
int thread_join(void)
{
	char* stack;
	int pid = join((void**)&stack);
	free(stack);
	return pid;
}

/*
 * Initializer for lock.
 * @param plock pointer to lock to initialize
 * @revisions
 *   GJE p4b - Created. Implementation: OSTEP, Arpaci-Dusseau.
 */
void lock_init(lock_t* plock)
{
	plock->ticket = 0;
	plock->turn = 0;
}


/*
 * Returns with exclusive possession of the lock
 * @param plock pointer to the lock
 * @revisions
 *   GJE p4b - Created. Implementation: OSTEP, Arpaci-Dusseau.
 */
//int lock(void) { return 1; }
//{
//	printf(1, "out\n");
//	plock->ticket++;
	/*
	int myturn = plock->ticket;
	// fetch_and_add(&plock->ticket, 1);
	while (plock->turn != myturn)
	{
		yield();
	}
	*/
	//return;
//}

/*
 * Release the held lock
 * @param plock pointer to lock to release
 * @revisions
 *   GJE p4b - Created. Implementation: OSTEP, Arpaci-Dusseau.
 */
void lock_release(lock_t* plock)
{
	plock->turn++;
}
