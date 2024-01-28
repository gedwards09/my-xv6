/*
 * Declarations for uthread library functions
 * @revisions
 *   GJE p4b - Created
 */

typedef struct __lock_t
{
	int ticket;
	int turn;
} lock_t;

int thread_create(void (*)(void*, void*), void*, void*);
int thread_join(void);
void lock_init(lock_t* plock);
void lock_acquire(lock_t* plock);
void lock_release(lock_t* plock);
