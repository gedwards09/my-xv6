/*
 * Created three proceses with 10, 20, and 30 tickets respectively
 * and graphs the number of ticks scheduled for each process
 * to demonstrate the lottery scheduler
 *
 * @author Greg Edwards
 * @version 1.0
 * @revisions
 *   GJE p2b - Created
 */

#include "types.h"
#include "user.h"
#include "pstat.h"
#include "rand.h"

#define pstatErrorMsg() "error:lottery.c:could not get process info\n"
#define GetProcessInfo(pstat) if(getpinfo(&pstat) < 0)\
							   { printf(1, pstatErrorMsg()); exit(); }

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf(1, "lottery.c:usage \"lottery spins\"\n");
		exit();
	}

	struct pstat pstat;
	
	char* pargv[3];
	pargv[0] = "spin";
	pargv[1] = argv[1];
	pargv[2] = 0;

	int n = 3;
	int tickets[] = {10, 20, 30};
	int pid[n];
	for (int i=0; i < n; i++)
	{
		pid[i] = fork();
		if (pid[i] == 0)
		{
			settickets(tickets[i]);
			exec("spin", pargv);
			exit();
		}
	}

	settickets(20);

	printf(1, "\tA:%d:%d\tB:%d:%d\tC:%d:%d\n", 
			pid[0], tickets[0],
			pid[1], tickets[1],
			pid[2], tickets[2]);

	for (int k=0; k < 50; k++)
	{

		GetProcessInfo(pstat);
		printf(1, "%d", k);
		for (int i=0; i < n; i++)
		{
			for (int j=0; j < NPROC; j++)
			{
				if (pstat.pid[j] == pid[i])
				{
					printf(1, "\t%d", pstat.ticks[j]);
					break;
				}
			}
		}
		printf(1, "\n");
		yield();
	}

	for (int i=0; i < n; i++)
	{
		kill(pid[i]);
	}
	while (wait() != -1) {  }
	exit();
}
