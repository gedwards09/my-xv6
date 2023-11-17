/*
 * This program forks into two processes: A parent process which does nothing, 
 * and a child process which issues a getppid system call and prints out
 * the pid of the parent 
 *
 * @author: Greg Edwards
 * @version: 1.0
 * @revisions:
 *   GJE p1b - Created
 */

#include "types.h"
#include "user.h"

int main(void)
{
	int pid;
	int ppid;

	ppid = getpid();
	pid = fork();

	if (pid > 0)
	{
		printf(1, "parent: waiting...\n");
		pid = wait();
		printf(1, "parent: child %d is done\n", pid);
		printf(1, "parent: parent=%d exiting\n", ppid);
	}
	else if (pid == 0)
	{
		pid = getpid();
		ppid = getppid();
		printf(1, "child: parent=%d\n", ppid);
		printf(1, "child: child=%d exiting\n", pid);
		exit();
	}
	else
	{
		printf(1, "fork error\n");
	}
	exit();
}
