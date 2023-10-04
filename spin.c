/*
 * Creates a process with specified number of tickets to run for a specified 
 * number of iterations
 *
 * @author Greg Edwards
 * @version 1.0
 * @revisions
 *   GJE p2b - Created
 */

#include "types.h"
#include "user.h"

int main(int argc, char* argv[])
{
	int i;
	int ct = 0;

	if (argc == 1 || argc > 3)
	{
		exit();
	}
	else if (argc == 3)
	{
		if (settickets(atoi(argv[2])) < 0)
		{
			printf(1, "error:spin.c:could not set tickets\n");
			exit();
		}
	}

	for (i=0; i < atoi(argv[1]); i++)
	{
		ct += i;
	}
	exit();
}
