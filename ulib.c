/*
 * @revisions
 *   GJE p4b - Add thread and mutex lock library functions
 */

#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#define PGSIZE (4096)

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
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
void lock_aquire(lock_t* plock)
{
	plock->ticket++;
	int myturn = plock->ticket;
	// fetch_and_add(&plock->ticket, 1);
	while (plock->turn != myturn)
	{
		yield();
	}
}

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
