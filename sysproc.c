/*
 * @revisions
 *   GJE p1b - add sys_getppid syscall
 *   GJE p2b - add sys_settickets syscall
 *           - add sys_getpinfo syscall
 *           - add sys_yield syscall
 *   GJE p3b - add sys_mprotect syscall
 *           - add sys_munprotect syscall
 */

#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

/*
 * System call to get PID of parent 
 * @returns PID of parent process
 * @revisions
 * GJE p1b - created
 */
int sys_getppid(void)
{
	return myproc()->parent->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

/*
 * System call to set the number of tickets for a process.
 * @returns 0 if sucessful, -1 otherwise
 * @revisions
 *   GJE p2b - Created
 */
int sys_settickets(void)
{
	int n;

	if (argint(0, &n) < 0)
	{
		return -1;
	}
	return settickets(n);
}

/*
 * System call to report info for all running processes
 * @returns 0 if sucessful, -1 otherwise
 * @revisions
 *   GJE p2b - Created
 */
int sys_getpinfo(void)
{
	char* p;
	
	if (argptr(0, &p, sizeof(struct pstat)) < 0)
	{
		return -1;
	}
	return getpinfo((struct pstat*) p);
}

/*
 * Handle system call for process to yield
 * @returns 0
 * Revisions
 *   GJE p2b - Created
 */
int sys_yield(void)
{
	yield();
	return 0;
}

int sys_mprotect(void)
{
	char* uva;
	int n;
	struct proc* curproc = myproc();

	if (argptr(0, &uva, sizeof(char)) < 0)
	{
		return -1;
	}
	if (((uint)uva) % PGSIZE != 0)
	{
		return -1;
	}
	if (argint(1, &n) < 0)
	{
		return -1;
	}
	if (n <= 0)
	{
		return -1;
	}

	for( int i = 0; i < n; i++)
	{
		clearptew(curproc->pgdir, uva + i * PGSIZE);
	}

	return 0;
}

int sys_munprotect(void)
{
	char* uva;
	int n;
	struct proc* curproc = myproc();

	if (argptr(0, &uva, sizeof(char)) < 0)
	{
		return -1;
	}
	if (((uint)uva) % PGSIZE != 0)
	{
		return -1;
	}
	if (argint(1, &n) < 0)
	{
		return -1;
	}
	if (n <= 0)
	{
		return -1;
	}

	for( int i = 0; i < n; i++)
	{
		permitptew(curproc->pgdir, uva + i * PGSIZE);
	}

	return 0;
}
