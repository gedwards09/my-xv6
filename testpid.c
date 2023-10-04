/*
 * This program forks into two processes: A child process which does nothing, 
 * and a parent process which waits for child to return then prints the pid of
 * the child.
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
	int ppid = getpid();
	int pid = fork();

	if (pid > 0)
	{
		printf(1, "parent: waiting...\n");
		pid = wait();
		printf(1, "parent: child %d is done\n", pid);
		printf(1, "parent: parent=%d\n", ppid);
	}
	else if (pid == 0)
	{
		ppid = getppid();
		printf(1, "child: child=%d\n",pid);
		printf(1, "child: parent=%d\n", ppid);
		exit();
	}
	else
	{
		printf(1, "fork error\n");
	}
	exit();
}
