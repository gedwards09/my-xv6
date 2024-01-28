/*
 * @revisions
 *   GJE p1b - add getppid syscall
 *           - add getreadcount syscall
 *   GJE p2b - add settickets syscall
 *           - add getpinfo syscall
 *           - add yield syscall
 *   GJE p3b - add mprotect syscall
 *           - add munprotect syscall
 *   GJE p4b - add clone syscall
 *           - add join syscall
 */

#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(getppid)
SYSCALL(getreadcount)
SYSCALL(settickets)
SYSCALL(getpinfo)
SYSCALL(yield)
SYSCALL(mprotect)
SYSCALL(munprotect)
SYSCALL(clone)
SYSCALL(join)
