/*
 * @revisions
 *   GJE p2b - implement lottery scheduler
 *   GJE p4b - add clone()
 *           - add join()
 *           - only wait() for processes with separate address space
 *           - grow all threads sharing address space when increasing one
 */

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "pstat.h"
#include "rand.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
/*
 * @revisions
 *   GJE p2b - set initial process ticket count to 1
 */
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;
  p->tickets = 1;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
/*
 * @revisions
 *   GJE p4b - Increase size of all threads sharing address space when 
 *             increasing one
 */
int
growproc(int n)
{
	uint sz;
  	struct proc *curproc = myproc();
 	struct proc *p;

  	sz = curproc->sz;
  	if(n > 0){
  	  if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
  	    return -1;
  	} else if(n < 0){
  	  if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
  	    return -1;
  	}
	curproc->sz = sz;
  	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  	{
  		if (p->pgdir == curproc->pgdir)
  	  	{
  	  		p->sz = sz;
  	  	}
  	}
  	switchuvm(curproc);
  	return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
/*
 * @revisions
 *   GJE p2b - set ticket count of child process equal to parent
 */
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;
  np->tickets = curproc->tickets;
  np->ticks = 0;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() or join() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait() or join().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
/*
 * @revisions
 *   GJE p4b - Only wait for processes with separate address space.
 */
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc || p->pgdir == curproc->pgdir)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

/*
 * Creates a new process sharing the same address space as the calling process.
 * @param fcn Pointer to function to execute
 * @param arg1 First argument passed to fcn
 * @param arg2 Second argument passed to fcn
 * @param stack User stack for the new process.
 *              One page in size and page aligned.
 * @returns the pid of the newly created thread if successful, -1 otherwise
 * @revisions
 *   GJE p4b - Created
 */
int clone(void (*fcn)(void*, void*), void* arg1, void* arg2, void* stack)
{
	int i;
  	int pid;
  	struct proc *np;
  	struct proc *curproc = myproc();
  	uint ustack[CLONE_NARGS];
  	uint sp = (uint)stack + PGSIZE;

  	// Allocate process.
  	if((np = allocproc()) == 0){
  	  return -1;
  	}

  	// push arguments to stack
  	ustack[0] = 0xffffffff;// fake return PC
  	ustack[1] = (uint)arg1;
  	ustack[2] = (uint)arg2;

  	sp -= sizeof(ustack);
  	if (copyout(curproc->pgdir, sp, ustack, sizeof(ustack)) < 0)
  	{
		return -1;
	}

  	// point to SAME address space as current porc
  	np->pgdir = curproc->pgdir;

  	np->sz = curproc->sz;
  	np->parent = curproc;
  	*np->tf = *curproc->tf;
  	np->tickets = curproc->tickets;
  	np->ticks = 0;

  	// setup new user stack and registers
	np->tf->eax = 0; // clear eax so 0 returned to child
  	np->tf->ebp = (uint)stack;
  	np->tf->eip = (uint)fcn;
  	np->tf->esp = sp;

  	for(i = 0; i < NOFILE; i++)
  	  if(curproc->ofile[i])
  	    np->ofile[i] = filedup(curproc->ofile[i]);
  	np->cwd = idup(curproc->cwd);

  	safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  	pid = np->pid;

  	acquire(&ptable.lock);

  	np->state = RUNNABLE;

  	release(&ptable.lock);

  	return pid;
}

/*
 * Waits for thread sharing the same address space to exit.
 * @param stack OUTPUT pointer to exited thread's user stack.
 *                     Responsibility of the calling process to free.
 * @returns pid of exited thread if successful, -1 otherwise
 * @revisions
 *   GJE p4b - Created
 */
int join(void** stack)
{
  struct proc *p;
  int hasthreads, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited threads
    hasthreads = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc || p->pgdir != curproc->pgdir)
        continue;
      hasthreads = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
		// 
		*stack = (void*)PGROUNDDOWN(p->tf->esp);
        kfree(p->kstack);
        p->kstack = 0;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any threads.
    if(!hasthreads || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    sleep(curproc, &ptable.lock);
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
/*
 * @revisions
 *   GJE p2b - Implement lottery scheduler
 */
void
scheduler(void)
{
	struct proc* p;
	struct cpu *c = mycpu();
	c->proc = 0;
  
	for(;;)
	{
		// Enable interrupts on this processor.
		sti();

		// Loop over process table looking for process to run.
		acquire(&ptable.lock);
		
		p = getLotteryWinner();

		if (p != 0)
		{
			// Switch to chosen process.  It is the process's job
			// to release ptable.lock and then reacquire it
			// before jumping back to us.
			c->proc = p;
			switchuvm(p);
			p->state = RUNNING;

			//audit count of ticks given to each process
			p->ticks++;

			swtch(&(c->scheduler), p->context);
			switchkvm();

			// Process is done running for now.
			// It should have changed its p->state before coming back.
			c->proc = 0;
		}	
		release(&ptable.lock);
	}
}

/*
 * Picks a lottery schedule winner based on tickets allocated to each process
 * @returns Pointer to winning process if sucessful, 0 if unsuccessful
 * @revisions
 *   GJE p2b - Created
 */
struct proc* getLotteryWinner(void)
{
	struct proc* p;
	int ticketCount = 0;
	int winner;
	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
	{
		if (p->state == RUNNABLE)
		{
			ticketCount += p->tickets;
		}
	}

	if (ticketCount == 0)
	{
		return 0;
	}

	winner = rand0() % ticketCount;

	ticketCount = 0;
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
	{
		if (p->state == RUNNABLE)
		{
			ticketCount += p->tickets;
			if (ticketCount > winner)
			{
				return p;
			}
		}
	}
	return 0;
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

/*
 * Report ticket info for all running processes
 * @param pstat Pointer to pstat struct to file data for running processes
 * @returns 0 if sucessfull, -1 otherwise
 * @revisions
 *   GJE p2b - Created
 */
int getpinfo(struct pstat* pstat)
{
	struct proc* p;
    for(int i=0; i < NPROC; i++)
	{
		p = &ptable.proc[i];
		if (p->state == UNUSED)
		{
			pstat->inuse[i] = 0;
		}
		else
		{
			pstat->inuse[i] = 1;
			pstat->tickets[i] = p->tickets;
			pstat->pid[i] = p->pid;
			pstat->ticks[i] = p->ticks;
		}
	}

	return 0;
}


/*
 * Sets the number of tickets for the calling process
 * @param number The number of tickets to set for the process
 * @revisions
 *   GJE p2b - Created
 */
int settickets(int number)
{
	if (number <= 0)
	{
		return -1;
	}
	myproc()->tickets = number;
	return 0;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
