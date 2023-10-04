/*
 * Definiton of pstat struct
 * 
 * @author Greg Edwards
 * @version 1.0
 * @revisions
 *   GJE p2b - Created
 */

#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat
{
	/* whether this slot of the process table is in use (1 or 0) */
	int inuse[NPROC];
	/* the number of tickets this process has */
	int tickets[NPROC];
	/* the PID of each process */
	int pid[NPROC];
	/* the number of ticks each process has accumulated */
	int ticks[NPROC];
};

#endif // _PSTAT_H_
