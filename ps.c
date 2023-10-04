/*
 * Implementation of ps shell routine
 * 
 * @author: Greg Edwards
 * @version: 1.0
 * @revisions:
 *   GJE p2b - Created
 */

#include "types.h"
#include "param.h"
#include "pstat.h"
#include "user.h"

int main(void)
{
	struct pstat p;
	if (getpinfo(&p) < 0)
	{
		printf(1, "Error:ps.c: Could not get process info\n");
		exit();
	}

	printf(1, "\tPID\tTickets\tTicks\n");
	for (int i=0; i < NPROC; i++)
	{
		if (p.inuse[i] == 1)
		{
			printf(1, "\t%d\t%d\t%d\n", p.pid[i], p.tickets[i], p.ticks[i]);
		}
	}
	exit();
}
